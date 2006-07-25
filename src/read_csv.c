/* read_csv.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site,
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
 *
 * Contributing author of csv parsing code: Michael Lawrence
*/

/* This file contains code written by Paul Hsieh (specifically the csv_row_parse()
 * function and related code). His license demands the inclusion of the below 
 * copyright notice, conditions, and disclaimer. It also probably contains some 
 * code by Dongshin Kim, a student who worked with Di Cook at ISU and original 
 * author of the GGobi csv parsing code. 
*/

/* bcsv - read comma separated value format
 * Copyright (c) 2003 Paul Hsieh <qed@pobox.com>
 * Modified by Michael Lawrence (c) 2005 to depend on GLib instead of bstring
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 *     Redistributions of source code must retain the above copyright notice, 
 *     this list of conditions and the following disclaimer. 
 * 
 *     Redistributions in binary form must reproduce the above copyright 
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution. 
 * 
 *     Neither the name of bcsv nor the names of its contributors may be 
 *     used to endorse or promote products derived from this software without 
 *     specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include "vars.h"
#include "externs.h"
#include "string.h"
#include "ggobi-stage.h"

/* Error conditions */
#define ERR_GARBAGE_AROUND_QUOTE (-1)
#define ERR_NO_ENTRY             (-2)
#define ERR_NON_ENCLOSED_QUOTE   (-3)
#define ERR_DANGLING_QUOTE       (-4)
#define ERR_STREAM_READ          (-5)

#ifdef DISALLOW_PARADOX_QUOTE_ERRORS
#define crpStrictAssert(cond,error) g_return_val_if_fail (cond, error)
#else
#define crpStrictAssert(cond,error)
#endif

#define fastIsSpace(x) (isSpaceTable[128+(x)])
static gchar isSpaceTable[128 + 256];

struct RowEntry
{
  gint ofs, len;
};

typedef struct _Row
{
  GString *src;
  gint rIdx;
  struct RowEntry *entry;
} Row;

static int csv_row_parse (Row * row, GIOChannel * channel, gint trim);
static gboolean has_row_labels (GList * rows);
static void load_column_labels (Row * row, GGobiStage * d,
                                gboolean row_labels);
static void load_row_labels (GList * rows, GGobiStage * d,
                             gboolean has_labels);
static void load_row_values (GList * rows, GGobiStage * d,
                             gboolean row_labels);
static void tokenize_row (Row * row);
GSList *read_csv_data (InputDescription * desc, ggobid * gg);
static void row_free (Row * r);

/* This code would probably be simpler if it used GScanner */

static int
csv_row_parse (Row * row, GIOChannel * channel, gint trim)
{
  gint i = 0, startOfs = 0, k, QuoteAfterQuote = 0;
  gint rowMlen;

  if (row == NULL || channel == NULL)
    return 0;

  row->rIdx = 0;
  row->src = g_string_new ("");

  if (g_io_channel_read_line_string (channel, row->src, NULL, NULL) !=
      G_IO_STATUS_NORMAL) {
    return ERR_STREAM_READ;
  }

  g_string_append_c (row->src, '\r');

  rowMlen = 4;                  /* Set smaller if this leads to too much memory usage */
  row->entry = g_new (struct RowEntry, rowMlen);
  if (row->entry == NULL) {
    return 0;
  }

  goto LParseNewEntry;

LPostProcessQuote:             /* Post-process quote enclosed strings */
  {
    gint ll, l, m;
    guchar *v;
    ll = l = row->entry[row->rIdx].len;
    v = row->src->str + row->entry[row->rIdx].ofs;
    m = k = 1;
    for (m = k = 1; m < l; k++, m++) {
      if (v[m] == '"' && v[m - 1] == '"') {
        /* Skip the second of two consecutive " characters */
        ll--;
        m++;
      }
      v[k] = v[m];
    }
    row->entry[row->rIdx].len = ll;
  }

  /* Check for pending exit condition */
  if (row->src->str[i] == '\r' || row->src->str[i] == '\n') {
    goto LDoneLast;
  }

  QuoteAfterQuote = 0;

LCommaEncountered:             /* , encountered */

  i++;
  row->rIdx++;                  /* New row entry */

  if (row->rIdx >= rowMlen) {
    struct RowEntry *t;
    rowMlen += rowMlen;
    t = g_renew (struct RowEntry, row->entry, rowMlen);
    if (t == NULL) {
      g_free (row->src);
      row->rIdx = 0;
      g_free (row->entry);
      return 0;
    }
    row->entry = t;
  }

LParseNewEntry:                /* End of , or beginning */

  k = i;
  while (fastIsSpace (row->src->str[i]))
    i++;

  if (row->src->str[i] == '"') {
    /* Single quote encountered */
    i++;
    startOfs = i;               /* Will be useful for the triple quote case. */
    row->entry[row->rIdx].ofs = i;
    g_return_val_if_fail (i < row->src->len, ERR_DANGLING_QUOTE);
    goto LAfterOpenQuote;
  }

  /* If there is no trimming, then consumption is obvious */

  if (!trim) {
    row->entry[row->rIdx].ofs = k;
    while (row->src->str[i] != ',') {
      crpStrictAssert (row->src->str[i] != '"', ERR_GARBAGE_AROUND_QUOTE);
      i++;
      if (row->src->str[i] == '\r' || row->src->str[i] == '\n') {
        row->entry[row->rIdx].len = i - k;
        goto LDoneLast;
      }
    }
    row->entry[row->rIdx].len = i - k;
    goto LCommaEncountered;
  }

  row->entry[row->rIdx].ofs = i;
  if (row->src->str[i] == ',') {
    row->entry[row->rIdx].len = 0;
    goto LCommaEncountered;
  }

  startOfs = i;
  k = i;

  /* Middle unquoted characters */

  while (row->src->str[i] != ',') {
    if (row->src->str[i] == '\r' || row->src->str[i] == '\n') {
      row->entry[row->rIdx].len = k - startOfs;
      /* A single empty entry should be explicitely given in quotes */
      if (row->rIdx == 0 && k == startOfs)
        return 1;
      goto LDoneLast;
    }
    i++;
    k = i;
    /* Consume whatever spaces there are */
    while (fastIsSpace (row->src->str[i]))
      i++;
    crpStrictAssert (row->src->str[i] != '"', ERR_NON_ENCLOSED_QUOTE);
  }
  row->entry[row->rIdx].len = k - startOfs;
  goto LCommaEncountered;

LAfterOpenQuote:               /* Characters after an open quote */

  while (1) {

    while (row->src->str[i] != '"') {

    LInsideQuote:
      i++;

      /* Go by length, and potentially read in more lines, since
         the \r character may be embedded in the quoted string */
      if (i >= row->src->len) {
        GString *tmp_str = g_string_new ("");
        if (g_io_channel_read_line_string (channel, tmp_str, NULL, NULL) !=
            G_IO_STATUS_NORMAL) {
          g_string_free (tmp_str, TRUE);
          return ERR_STREAM_READ;
        }
        g_return_val_if_fail (tmp_str->len > 0, ERR_DANGLING_QUOTE);
        g_string_append (row->src, tmp_str->str);
        g_string_free (tmp_str, TRUE);
      }
    }

    /* Quote after characters after an open quote */

    k = i;
    i++;
    if (row->src->str[i] == '\r' || row->src->str[i] == '\n') {
      row->entry[row->rIdx].len = k - startOfs;
      if (QuoteAfterQuote)
        goto LPostProcessQuote;
      goto LDoneLast;
    }

    if (row->src->str[i] == '"') {
      QuoteAfterQuote = 1;
      goto LInsideQuote;
    }

    while (fastIsSpace (row->src->str[i]))
      i++;
    crpStrictAssert (row->src->str[i] == ',', ERR_GARBAGE_AROUND_QUOTE);
    if (row->src->str[i] == ',') {
      row->entry[row->rIdx].len = k - startOfs;
      if (QuoteAfterQuote)
        goto LPostProcessQuote;
      goto LCommaEncountered;
    }
  }
LDoneLast:
  row->rIdx++;
  return 1;
}

/* END BCSV CODE */

GSList *
read_csv (InputDescription * desc, ggobid * gg, GGobiPluginInfo * plugin)
{
  return (read_csv_data (desc, gg));
}

InputDescription *
read_csv_input_description (const char *const fileName,
                            const char *const modeName, ggobid * gg,
                            GGobiPluginInfo * info)
{
  InputDescription *desc;
  desc = (InputDescription *) g_malloc0 (sizeof (InputDescription));

  desc->fileName = g_strdup (fileName);
  desc->mode = csv_data;
  desc->desc_read_input = &read_csv;

  return (desc);
}


/*  Heuristic: If the first row has an empty in the first column and
	and all the values in the first column are unique, we have row names.
*/
static gboolean
has_row_labels (GList * rows)
{
  GHashTable *hash =
    g_hash_table_new ((GHashFunc) g_str_hash, (GEqualFunc) g_str_equal);
  Row *first = (Row *) rows->data;

  if (first->entry[0].len != 0)
    return false;

  while (rows) {
    Row *row = (Row *) rows->data;
    gchar *str = row->src->str + row->entry[0].ofs;
    if (g_hash_table_lookup (hash, str)) {
      g_warning ("Duplicate row name: %s - treating rownames as data", str);
      g_hash_table_destroy (hash);
      return false;
    }
    g_hash_table_insert (hash, str, str);
    rows = g_list_next (rows);
  }
  g_hash_table_destroy (hash);
  return true;
}

static void
load_column_labels (Row * row, GGobiStage * d, gboolean row_labels)
{
  gint i;
  gint offset = (row_labels ? 1 : 0);
  GGOBI_STAGE_VARIABLES_ITERATE(d) {
    if (row->entry[j + offset].len == 0)
      ggobi_stage_set_col_name(d, j, NULL);
    else
      ggobi_stage_set_col_name(d, j, row->src->str + row->entry[j + offset].ofs);
  }
}

static void
load_row_labels (GList * rows, GGobiStage * d, gboolean has_labels)
{
  if (!has_labels)
    return;

  for (gint i = 0; rows; rows = g_list_next (rows), i++) {
    Row *row = (Row *) rows->data;
    ggobi_stage_set_row_id(d, i, row->src->str + row->entry[0].ofs, false);    
  }
}

static void
load_row_values (GList * rows, GGobiStage * d, gboolean row_labels)
{
  gint i, j, offset = (row_labels ? 1 : 0);
  GList *cur;

  GGOBI_STAGE_VARIABLES_ITERATE(d) {
    for (cur = rows, i = 0; cur; cur = cur->next, i++) {
      Row *row = (Row *) cur->data;
      gchar *str = row->src->str + row->entry[j + offset].ofs;
      
      ggobi_stage_set_string_value(d, i, j, str);
    }
  }
}

static GGobiStage *
create_data (GList * rows, gchar * name)
{
  GGobiStage *d;
  guint nrows = g_list_length (rows), ncols = 0;

  gboolean row_labels = has_row_labels (rows);

  if (nrows > 0)
    ncols = ((Row *) rows->data)->rIdx;
  if (row_labels)
    ncols--;

  d = GGOBI_STAGE(ggobi_data_new (nrows - 1, ncols));
  ggobi_stage_set_name(d, name);

  load_column_labels ((Row *) rows->data, d, row_labels);
  rows = g_list_next (rows);    /* skip the column labels */

  load_row_labels (rows, d, row_labels);
  load_row_values (rows, d, row_labels);
  ggobi_data_add_attributes(GGOBI_DATA(d));

  return (d);
}

/* This makes things a lot easier - Michael */
static void
tokenize_row (Row * row)
{
  gint i;
  for (i = 0; i < row->rIdx; i++) {
    row->src->str[row->entry[i].ofs + row->entry[i].len] = '\0';
  }
}

GSList *
read_csv_data (InputDescription * desc, ggobid * gg)
{
  GGobiStage *d;
  GIOChannel *channel;
  gint ret;
  GList *rows = NULL;
  GSList *ds = NULL;

  memset (isSpaceTable, 0, sizeof (isSpaceTable));
  fastIsSpace ((gchar) ' ') = 1;
  fastIsSpace ((gchar) '\f') = 1;
  fastIsSpace ((gchar) '\t') = 1;
  fastIsSpace ((gchar) '\v') = 1;
  fastIsSpace ((guchar) ' ') = 1;
  fastIsSpace ((guchar) '\f') = 1;
  fastIsSpace ((guchar) '\t') = 1;
  fastIsSpace ((guchar) '\v') = 1;

  /* Open the file */
  if (!(channel = g_io_channel_new_file (desc->fileName, "r", NULL))) {
    return false;
  }

  /* Parse each row */
  do {
    Row *cur = g_new0 (Row, 1);
    ret = csv_row_parse (cur, channel, 1);
    if (ret >= 0) {
      tokenize_row (cur);
      rows = g_list_append (rows, cur);
    }
    else
      row_free (cur);
  } while (ret >= 0);

  /* Close the file */
  g_io_channel_shutdown (channel, FALSE, NULL);

  /* Load the parsed data into the GGobiStage */
  d = create_data (rows, desc->baseName);

  /* Cleanup */
  g_list_foreach (rows, (GFunc) row_free, NULL);
  g_list_free (rows);

  ds = g_slist_append (ds, d);
  return (ds);
}

static void
row_free (Row * row)
{
  g_string_free (row->src, true);
  if (row->entry)
    g_free (row->entry);
  g_free (row);
}

gboolean
isCSVFile (const gchar * fileName, ggobid * gg, GGobiPluginInfo * plugin)
{
  gchar *extension = strrchr (fileName, '.');
  return(extension && (!strcmp (extension, ".asc") || !strcmp (extension, ".txt") || !strcmp (extension, ".csv")));
}
