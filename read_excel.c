/* read_excel.c */
/* This code was written by Dongshin Kim, at Iowa State University
   under supervision by Dianne Cook */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"
#include "rb.h"

#define BLOCKSIZE 1000
#define INITSTRSIZE 512
#define DELIMITER 44 /* DELIMITER: ',' */

/* These static variables will have to go */
gboolean g_is_column = 0;
gboolean g_is_row = 0;
gint g_tmp;

gboolean name_set(FILE* fp, InputDescription *desc, datad* d, ggobid* gg);
void whatisfiletype(FILE* fp, gint* ncols, gint* nrows);
gboolean is_num(gchar* word);
void read_col_labels(datad* d,FILE* fp);
void read_row_labels(datad* d,FILE* fp);
gboolean read_excel(datad* d, FILE* fp, gint* text_table, Tree* text_category);
gboolean setup_category(datad* d, gint* text_table, Tree* text_category);
void InorderTravel_setup_category(Tree* T, Node* X, vartabled* vt);

void InorderTravel_setup_category(Tree* T, Node* X, vartabled* vt)
{
  if(X != T->NIL)
  {
    InorderTravel_setup_category(T, X->Left,vt);
    vt->level_counts[g_tmp] = 0;
    vt->level_values[X->index-1] = X->index;/*g_tmp+1; to fix order bug*/
    vt->level_names[X->index-1] = g_strdup(X->key);
    g_tmp++;
    InorderTravel_setup_category(T, X->Right,vt);
  }
}

gboolean setup_category(datad* d, gint* text_table, Tree* text_category)
{
  gint i, j;
  vartabled* vt;

  for(i=0;i<d->ncols;i++)
  {
    if(text_table[i] != 0)
    {
      /* this column is categorical column */
      vt = vartable_element_get (i, d);
      vt->vartype = categorical;
      vt->nlevels = text_table[i];
      vt->level_values = (gint *) g_malloc(text_table[i] * sizeof(gint));
      vt->level_counts = (gint *) g_malloc(text_table[i] * sizeof(gint));
      vt->level_names = (gchar **) g_malloc(text_table[i] * sizeof(gchar *));
      g_tmp = 0;
      InorderTravel_setup_category(&text_category[i],text_category[i].Root,vt);

      /* Debugging, dfs */
#ifdef DEBUG_READ_CSV
      if (vt->nlevels > 0) {
	g_printerr ("nlevels: %d\n", vt->nlevels);
	{
	  gint k;
	  for (k=0; k<vt->nlevels; k++)
	    g_printerr ("level_name: %s\n", vt->level_names[k]);
	}
      }
#endif



      for(j=0;j<d->nrows;j++)
      {
        gint inx;
        if (vt->nmissing && MISSING_P(j, i))
          continue;
        inx = (gint) d->raw.vals[j][i];
        vt->level_counts[inx-1]++; 
      }
    }
  }
  return true;
}
gboolean read_excel(datad* d, FILE* fp, gint* text_table, Tree* text_category)
{
  gint ch;
  gchar tmp[256];
  gint inx = 0;
  gint i,j;
  vartabled* vt;
  Node* node;

  /* Initialize floating point array */
  arrayf_alloc (&d->raw, d->nrows, d->ncols);
  /* Initialize short array */
  arrays_alloc_zero (&d->missing, d->nrows, d->ncols);

  rewind(fp);

  if(g_is_column)
  {
    /* Strip off one row */
    while(true)
    {
      ch = fgetc(fp);
      if((ch == 10)||(ch==13))
        break;
    }
  }

  for(i=0;i<d->nrows;i++)
  {
    if(g_is_row)
    {
      /* Jump to the next column */
      while(true)
      {
        ch = fgetc(fp);
        if(ch == DELIMITER)
        {
          break;
        }
      }
    }
    for(j=0;j<d->ncols;j++)
    {
      memset(tmp,'\0',256);
      inx = 0;

      /* This unit should be one column */
      while(true)
      {
        ch = fgetc(fp);
        if((ch == DELIMITER)||(ch == 10)||(ch == 13))
        {
          if(is_num(tmp))
          {
            /* This is numeric value */
            d->raw.vals[i][j] = (gfloat) atof (tmp);
          }
          else
          {
            /* should be treated as string */
            /* Check whether it is Not available */
            if (g_strcasecmp (tmp, "na") == 0 || strcmp (tmp, ".") == 0 || strcmp (tmp, "NA") == 0)
            {
              d->nmissing++;
              vt = vartable_element_get (j, d);
              vt->nmissing++;
              d->missing.vals[i][j] = 1;
              d->raw.vals[i][j] = 0.0;
            }
            else
            {
              /* initialize the linked list of labels */
              if (text_table[j] != 0)
              {
                node = Search(&text_category[j],text_category[j].Root, tmp);
                if(node!=text_category[j].NIL)
                {
                  /*d->raw.vals[i][j] = node->index;*/
                }
                else
                {
                  node = (Node*)malloc(sizeof(Node));
                  strcpy(node->key, tmp);
                  node->index = text_table[j]+1;
                  text_table[j] = node->index;
                  InsertFixup(&text_category[j],node);
                }
              }
              else /* already have a start, now fill in remaining categories */
              {
                node = (Node*)malloc(sizeof(Node));
                strcpy(node->key, tmp);
                node->index = text_table[j]+1;
                text_table[j] = node->index;
                InsertFixup(&text_category[j],node);
              }
              d->raw.vals[i][j] = node->index;
            }
          }
          break;
        }
        else if(ch == EOF)
        {
          return true;
        }
        else
        {
          tmp[inx] = ch;
          inx++;
        }
      }
    }
  }
  return true;
}

void read_row_labels(datad* d,FILE* fp)
{
  gint i;
  gint ch;
  gchar tmp[256];
  gchar* tmp2;
  gint inx = 0;
  gint missing_cnt = 0;
  memset(tmp,'\0',256);
  rowlabels_alloc (d);
  rewind(fp);

  if(g_is_row)
  {
    for(i=0;i<d->nrows;i++)
    {
      memset(tmp,'\0',256);
      /* strip one line */
      while(true)
      {
        ch = fgetc(fp);
        if((ch == 10)||(ch == 13))
        {
          break;
        }
        if(ch == EOF)
          return;
      }
      while(true)
      {
        ch = fgetc(fp);
        if(ch == DELIMITER)
        {
          if(tmp[0] != '\0')
          {
            tmp2 = g_strdup_printf("%s", tmp);
            g_array_append_val(d->rowlab, tmp2);
          }
          else
          {
            tmp2 = g_strdup_printf("Row %d", missing_cnt+1);
            g_array_append_val(d->rowlab, tmp2);
            missing_cnt++;
          }
          memset(tmp,'\0',256);
          inx = 0;
          break;
        }
        else
        {
          tmp[inx] = ch;
          inx++;
        }
      }
    }
  }
  else
  {
    for(i=0;i<d->nrows;i++)
    {
      tmp2 = g_strdup_printf("%d", i+1);
      g_array_append_val(d->rowlab, tmp2);
    }
  }
}

void read_col_labels(datad* d,FILE* fp)
{
  vartabled* vt;
  gchar tmp[256];
  gchar ch;
  gint i;
  gint col_inx = 0;
  gint inx = 0;
  gint missing_cnt = 0;

  rewind(fp);
  memset(tmp,'\0',64);
  if(g_is_row)
  {
    /* jump to the next column */
    while(true)
    {
      ch = fgetc(fp);
      if((ch == DELIMITER))
        break;
    }
  }
  if(g_is_column)
  {
    for(i=0;i<d->ncols;i++)
    {
      while(true)
      {
        ch = fgetc(fp);
        if((ch == DELIMITER)||(ch == 10)||(ch == 13))
        {
          if(tmp[0] == '\0')
          {
            vt = vartable_element_get (col_inx, d);
            vt->collab = g_strdup_printf("Col %d", missing_cnt+1);
            vt->nickname = g_strndup(vt->collab, 2);
      missing_cnt++;
          }
          else
          {
            vt = vartable_element_get (col_inx, d);
            vt->collab = g_strdup(tmp);
      vt->nickname = g_strndup(vt->collab, 2);
          }
          col_inx++;
          inx = 0;
          memset(tmp, '\0', 256);
          break;
        }
        else
        {
          tmp[inx] = ch;
          inx++;
        }
      }
    }
    for(i=0;i<d->ncols;i++)
    {
      vt = vartable_element_get (i, d);
      vt->collab_tform = g_strdup (vt->collab);
    }
  }
  else
  {
    for(i=0;i<d->ncols;i++)
    {
      vt = vartable_element_get (i, d);
      vt->collab = g_strdup_printf("Col %d", missing_cnt+1);
      vt->nickname = g_strndup(vt->collab, 2);
      missing_cnt++;
      vt->collab_tform = g_strdup (vt->collab);
    }
  }
}

gboolean is_num(gchar* word)
{
  gchar ch;
  gint inx = 0;
  gboolean ret = false;
  gboolean num = false;
  while(true)
  {
    ch = word[inx];
    if((ch != ' ')&&(ch != '\t'))
    {
      if(ch == '\0')
        return ret;
      else if((ch<'0') || (ch>'9'))
      {
        if((ch == '-') || (ch == '.'))
        {
          if(num == false)
          {
            ret = false;
          }
          else
          {
            ret = true;
          }
        }
        else
        {
          ret = false;
          return ret;
        }
      }
      else
      {
        num = true;
        ret = true;
      }
      inx++;
    }
    else
    {
      inx++;
    }
  }
}

void whatisfiletype(FILE* fp, gint* ncols, gint* nrows)
{
  /* Check if there are column labels */
  gint ch;
  gchar tmp[256];
  gint inx = 0;
  Tree treetmp;
  InitRB_Tree(&treetmp);
  *ncols = 0;
  *nrows = 0;
  memset(tmp, '\0', 256);

  g_is_row = g_is_column = false;

  /* dfs: trying to use different information to decide whether
     row labels are provided, so I can detect errors in the file
     structure */
  if (fgetc(fp) == DELIMITER) {
#ifdef DEBUG_READ_CSV
    g_printerr ("first character is a delimiter\n");
#endif
    g_is_row = true;
    g_is_column = true;
  } else {
    rewind(fp);
  }

  while(true)
  {
    ch = fgetc(fp);
    if(ch == DELIMITER)
    {
      *ncols = *ncols + 1;
      if(!g_is_column && !is_num(tmp))
      {
        /* at least one string value in the first row */
        g_is_column = true;
      }
      inx = 0;
      memset(tmp, '\0', 256);
    }
    else if((ch == 13)||(ch==10))
    {
      if(tmp[0] != '\0')
      {
        *ncols = *ncols + 1;
      }
      break;
    }
    else
    {
      tmp[inx] = ch;
      inx++;
    }
  }
#ifdef DEBUG_READ_CSV
  g_printerr ("column labels? %d ncols = %d\n", g_is_column, *ncols);
#endif

  rewind(fp);

  inx = 0;
  memset(tmp, '\0', 256);
  /*
  g_is_row = true;
  */

  while(true)
  {
    /* if column labels are present, skip the first row */
    if (g_is_column) {
      while(true)
      {
        ch = fgetc(fp);
        if((ch == 10)||(ch == 13))
        {
          break;
        }
        if(ch == EOF)
          return;
      }
    }

    while(true)
    {
      ch = fgetc(fp);
      if(ch == -1)
        return;
      if(ch != DELIMITER)
      {
        tmp[inx] = ch;
        inx++;
      }
      else
      {
        *nrows = *nrows + 1;
        break;
      }
    }
    /* now check it is row label or not */
    if(!is_num(tmp))
    {
      Node* node = NULL;
      node = Search(&treetmp,treetmp.Root, tmp);
      if(node!=GetNIL(&treetmp))
      {
        /* there is the same one-> not row label */
        if (g_is_row) {
          g_printerr ("The strings in the first column are not unique, so they can't be row labels.\nRemove the initial comma from the first line.\n");
          exit(0);
      }
        /* g_is_row = false; */
      }
      else
      {
        node = (Node*)malloc(sizeof(Node));
        strcpy(node->key, tmp);
        node->index = *nrows;
        InsertFixup(&treetmp,node);
      }
    }
    /*
    else
    {
      g_is_row = false;
    }
    */
    memset(tmp,'\0',256);
    inx=0;
  }

}


gboolean name_set(FILE* fp, InputDescription *desc, datad* d, ggobid* gg)
{
  gchar *sep = g_strdup_printf ("%c", G_DIR_SEPARATOR);
  gchar *name = NULL;
  gchar **words = g_strsplit ((const gchar *) desc->baseName,(const gchar *) sep, 0);
  gchar **p;
  if (!words)
    return (false);
  for (p=words; *p; p++)
  {
    if (**p)
    {
      name = p[0];
    }
  }
  /* Initialize name structure in datad */      
  d->name = (name != NULL && strlen(name)) > 0 ? g_strdup (name) : g_strdup(gg->input->baseName);
  d->nickname = g_strndup (d->name, 2);
  g_strfreev (words);

  return (true);
}

gboolean 
read_csv_data(InputDescription *desc, ggobid *gg)
{
  FILE *fp;
  datad *d;
  gint text_table[MAXNCOLS];
  Tree text_category[MAXNCOLS];
  gint ncols, nrows;
  gint i;
    
  /* Initialize datad structure */
  d = datad_new(NULL, gg);
  /* prepare for using rbtree */
  for(i=0;i<MAXNCOLS;i++)
  {
    InitRB_Tree(&text_category[i]);
    text_table[i] = 0;
  }
  
  if ( (fp = fopen(desc->fileName, "r")) != NULL)
  {
    /* Call naming function */
    name_set(fp, desc, d, gg);
  
    /* Determine file type, g_is_row, g_is_col
     * Calculate ncols, nrows */
    whatisfiletype(fp, &ncols, &nrows);
#ifdef DEBUG_READ_CSV
    g_printerr ("whatis.. returns %d %d\n", ncols, nrows);
#endif
    if(g_is_column)
      d->nrows = nrows;
    else
      d->nrows = nrows + 1;
    /*
    if(g_is_row)
      d->ncols = ncols - 1;
    else
    */
      d->ncols = ncols;
#ifdef DEBUG_READ_CSV
    g_printerr ("we conclude: %d %d\n", d->ncols, d->nrows);
#endif

    /* Initialize vartable */
    vartable_alloc(d);
    vartable_init(d);

    /* Read Columns */
    read_col_labels(d,fp);

    /* Read Rows */
    read_row_labels(d,fp);

    /* Dummy for future purpose */
    br_glyph_ids_alloc(d);
    br_glyph_ids_init(d,gg);
    br_color_ids_alloc(d,gg);
    br_color_ids_init(d,gg);
    br_hidden_alloc(d);
    br_hidden_init(d);

    /* Reading Data (Main part)*/
    read_excel(d,fp,text_table, text_category);  

    /* Dealing with categorical variable */
    setup_category(d, text_table, text_category);
  }
  fclose(fp);
  return (true);
}
