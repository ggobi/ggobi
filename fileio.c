#include "ggobi.h"

#include <stdlib.h>
#include <stdio.h>

#ifndef G_OS_WIN32
#include <unistd.h>
#include <sys/stat.h>
#else
#include <glib.h>
#include <io.h>
#endif

#include <string.h>

#include "GGobiAPI.h"

#define FILE_SEPARATOR '/'

#ifdef USE_XML
static char *XMLSuffixes[] = {"xml", "xml.gz","xmlz"};
#endif

char *ASCIISuffixes[]  = {"dat"};
char *BinarySuffixes[] = {"bin"};


#ifdef USE_XML
ExtensionList xmlFileTypes = {
  xml_data, 
  NULL,
  0
};
#endif

ExtensionList asciiFileTypes = {
  ascii_data,
  NULL,
  0
};

ExtensionList binaryFileTypes = {
  binary_data,
  NULL, 
  0
};

GSList *FileTypeGroups = NULL;


gboolean isURL(const gchar *fileName);


InputDescription*
fileset_generate(const char *fileName, DataMode guess)
{
  InputDescription *desc = (InputDescription *)
    calloc(1, sizeof(InputDescription));
#ifndef G_OS_WIN32
  struct stat buf;
#else
  int ft=0;
#endif
  int i, j;
  int numGroups;
  GSList *groups;

  if(FileTypeGroups == NULL)
    initFileTypeGroups();

  groups = FileTypeGroups;

#ifndef G_OS_WIN32
  if(stat(fileName, &buf) != 0) {
#else
  if(access(fileName, ft) != 0){
#endif
    if(isURL(fileName)) {
      desc->mode = xml_data;
      desc->fileName = g_strdup(fileName);
      completeFileDesc(fileName, desc);
      return(desc);
    }

   /* Ok, so the user didn't give a full name and wants us to guess.  
      So we will look through the collections of format types and within
      each of these search through the different extensions for that type.
      If such a file exists, we assume it is of that format.
    */
    numGroups = g_slist_length(groups);
    if(guess == unknown_data) {
      for(i = 0; i < numGroups ; i++) {
        char buf[1000];
        ExtensionList *group;
        group = (ExtensionList *) g_slist_nth_data(groups, i);
        for(j = 0; j < group->len; j++) {
          if(group->extensions[j] && group->extensions[j][0])
            sprintf(buf, "%s.%s", fileName, group->extensions[j]);
          else
            sprintf(buf, "%s", fileName);

          if(check_file_exists(buf)) {
            guess = group->mode;
            desc->mode = guess;
            desc->fileName = g_strdup(buf);
            desc->baseName = g_strdup(fileName);
            desc->givenExtension = g_strdup(group->extensions[j]);
            break;
          }
        }
        if(guess != unknown_data)
          break;
      }
    } else {
      /* Attempt to find a file of this type. */
      /*     guess = unknown_data; */
      for(i = 0; i < numGroups ; i++) {
        ExtensionList *group;
        group = (ExtensionList *) g_slist_nth_data(groups, i);
        if(group->mode != guess) 
          continue;

        for(j = 0; j < group->len; j++) {
          char buf[1000];
          if(endsWith(fileName, group->extensions[j])) {
            g_printerr("%s does not exist!\n", fileName);
            return(NULL);
          }
          if(group->extensions[j] && group->extensions[j][0])
            sprintf(buf, "%s.%s", fileName, group->extensions[j]);
          else
            sprintf(buf, "%s", fileName);
          if(check_file_exists(buf)) {
            DataMode ok = verifyDataMode(buf, group->mode, desc);
            if(ok == guess && ok != unknown_data) {
              desc->fileName = g_strdup(buf);
              desc->baseName = g_strdup(fileName);
              desc->givenExtension = g_strdup(group->extensions[j]);
              desc->mode = guess;
              guess = group->mode;
              break;
            }
          }
        }
        if(desc->fileName)
           break;
      }
    }
 
    if(desc->fileName == NULL) {
      guess = unknown_data;
    }

  } else {
    desc->fileName = g_strdup(fileName);
    desc->mode = guess;
    desc->mode = guess = verifyDataMode(desc->fileName, desc->mode, desc);
    if(desc->mode == unknown_data) { 
      return(NULL);
    }
  }

  switch(guess) {
    case unknown_data:
      g_printerr("Cannot find a suitable file %s\n", fileName);
      return(NULL);
      break;
    default:
      break;
  }

  completeFileDesc(desc->fileName, desc);
  return(desc);
}

gchar *
completeFileDesc(const char *fileName, InputDescription *desc)
{
  int n;
  char *tmp = strrchr(fileName, '.');

  ExtensionList *group = getInputDescriptionGroup(desc->mode);
 
  if(group) {
    int i;
    const char *ext;
    n = strlen(fileName);
    for(i = 0;i<group->len; i++) {
      ext = group->extensions[i];
      if(endsWith(fileName, ext)) {
        int nbytes = strlen(fileName)-strlen(ext);
        desc->baseName = (char *)g_malloc(sizeof(char) * nbytes);
        g_snprintf(desc->baseName, nbytes, "%s", fileName);
        desc->givenExtension = g_strdup(ext);
        break;
      }
    }
  }
#if 0
  if(tmp) {
    desc->givenExtension = g_strdup(tmp+1);
    n = (tmp - fileName) + 1;
    desc->baseName = g_malloc(sizeof(char) * n);
    g_snprintf(desc->baseName, n, "%s", fileName);
  } else {
    desc->baseName = g_strdup(fileName);
  }
#endif

       /* Now compute the directory name. */
  tmp = strrchr(desc->baseName, FILE_SEPARATOR);
  if(tmp) {
    n = (tmp - desc->baseName) + 1;
    desc->dirName = g_malloc(sizeof(char) * n);
    g_snprintf(desc->dirName, n, "%s", desc->baseName);
  } else {
    desc->dirName = g_strdup(desc->baseName);
  }

  return(tmp);
}

DataMode
verifyDataMode(const char *fileName, DataMode mode, InputDescription *desc)
{
  switch(mode) {
#ifdef USE_XML
    case xml_data:
    if(!isXMLFile(fileName, desc))
        mode = unknown_data;
     break;
#endif
    case ascii_data:
    if(!isASCIIFile(fileName))
        mode = unknown_data;
     break;
     default:        
       mode = guessDataMode(fileName, desc);
  }

  return(mode);
}

DataMode
guessDataMode(const char *fileName, InputDescription *desc)
{
  FILE * f;

  f = fopen(fileName,"r");
  if(f == NULL)
    return(unknown_data);

#ifdef USE_XML
  if(isXMLFile(fileName, desc))
    return(xml_data);
#endif

  if(isASCIIFile(fileName))
    return(ascii_data);

  return(unknown_data);
}

#ifdef USE_XML
gboolean
isXMLFile(const char *fileName, InputDescription *desc)
{
  FILE * f;
  gint c;

  char *tmp = strrchr(fileName, '.');
  if(tmp && (strcmp(tmp, ".xmlz") == 0 || strcmp(tmp, ".gz") == 0 )) {
    desc->canVerify = false;
    return(true);
  }


  f = fopen(fileName,"r");
  if(f == NULL)
    return(false);

  desc->canVerify = true;
  while((c = getc(f)) != EOF) {
    if(c == ' ' || c == '\t' || c== '\n')
      continue;
    if(c == '<') {
      char buf[10];
      fgets(buf, 5, f);
      fclose(f);
      if(strcmp(buf,"?xml") == 0) {
        return(true);
      } else
        return(false);
    }
  }

  return(false);
}
#endif

gboolean
isASCIIFile(const char *fileName)
{
  FILE * f;
  double val;
  f = fopen(fileName,"r");
  if(f == NULL)
    return(false);

  if(fscanf(f, "%lf", &val) == 0) {
    return(false);
  } 

  return(true);
}

#ifdef G_OS_WIN32
gboolean
check_file_exists(const char *fileName)
{
  gint i=0;
  return (access(fileName, i) == 0);
}
#else
gboolean
check_file_exists(const char *fileName)
{
  struct stat buf;
  return (stat(fileName, &buf) == 0);
}
#endif

GSList *
initFileTypeGroups(void)
{
  FileTypeGroups = g_slist_alloc();

#ifdef USE_XML
  xmlFileTypes.extensions = XMLSuffixes;
  xmlFileTypes.len = 3;
#endif

  asciiFileTypes.extensions = ASCIISuffixes;
  asciiFileTypes.len = 1;


  binaryFileTypes.extensions = BinarySuffixes;
  binaryFileTypes.len = 1;

#ifdef USE_XML
  FileTypeGroups->data = (void *)&xmlFileTypes;
  g_slist_append(FileTypeGroups, &asciiFileTypes);
#else
  FileTypeGroups->data = (void *)&asciiFileTypes;
#endif

  g_slist_append(FileTypeGroups, &binaryFileTypes);

  return(FileTypeGroups);
}


ExtensionList *
getInputDescriptionGroup(DataMode mode)
{
  GSList *groups = FileTypeGroups;
  ExtensionList *el;
  while(groups) {
    el = (ExtensionList *) groups->data;
    if(el == NULL)
      return(NULL);

    if(el->mode == mode)
      return(el);

    groups = groups->next;
  }

  return(NULL);
}


gboolean
endsWith(const char *str, const char *what)
{
  return(strcmp(str+ strlen(str) - strlen(what), what) == 0);  
}


void
showInputDescription(InputDescription *desc, ggobid *gg)
{
  FILE *out = stderr;
  int i;
  fprintf(out, "Input File Information:\n");
  fprintf(out, "\tFile name: %s  (extension: %s)\n",
    desc->fileName, desc->givenExtension);
  fprintf(out, "\tDirectory: %s\n", desc->dirName);
  fprintf(out, "\tFormat: %s (%d), verified: %s\n",
    GGOBI(getDataModeDescription)(desc->mode), desc->mode, 
                                               desc->canVerify ? "yes" : "no");

  if(desc->extensions) {
    fprintf(out, "Auxillary files\n");
    for(i= 0; i < g_slist_length(desc->extensions) ; i++) {
      fprintf(out, "  %d) %s\n",
        i, (gchar *) g_slist_nth_data(desc->extensions, i));
    }
  }
  fflush(out);
}


gchar *
findAssociatedFile(InputDescription *desc, const gchar * const *extensions, int numExtensions, gint *which, gboolean isError)
{
  int i;
  char buf[100];

  if(desc->fileName == NULL || desc->fileName[0] == '\0' || strcmp (desc->fileName, "stdin") == 0) {
    return(NULL);
  }  

  for(i = 0; i < numExtensions; i++) {
    if(extensions[i] && extensions[i][0])
      sprintf(buf, "%s.%s", desc->baseName, extensions[i]);
    else
      sprintf(buf, "%s", desc->baseName);

    if(check_file_exists(buf)) {
      if(which)
        *which = i;
      return(g_strdup(buf));
    }
  }

  return(NULL);
}


int
addInputSuffix(InputDescription *desc, const gchar *suffix)
{
  if(desc->extensions) {
    g_slist_append(desc->extensions, g_strdup(suffix));
  } else {
    desc->extensions = g_slist_alloc();
    desc->extensions->data = g_strdup(suffix);
  }

  return(g_slist_length(desc->extensions));
}

int 
addInputFile(InputDescription *desc, const gchar *file) 
{
  return(addInputSuffix(desc, file));  
}


gboolean
isURL(const gchar *fileName)
{
 return((strncmp(fileName, "http:", 5) == 0 || strncmp(fileName, "ftp:", 4) == 0));
}
