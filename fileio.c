#include "ggobi.h"

#include <stdlib.h>
#include <stdio.h>

#ifndef WIN32
#include <unistd.h>
#include <sys/stat.h>
#else
#include <glib.h>
# ifdef __STRICT_ANSI__
# undef   __STRICT_ANSI__
# endif
# include <io.h>
#endif

#include <string.h>

#include "GGobiAPI.h"

static gchar *XMLSuffixes[] = { "xml", "xml.gz", "xmlz" };

#ifdef SUPPORT_PLUGINS
#include "plugin.h"
#endif

gchar *ASCIISuffixes[] = { "dat" };
gchar *BinarySuffixes[] = { "bin" };


ExtensionList xmlFileTypes = {
  xml_data,
  NULL,
  0
};

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

/*--------------------------------------------------------------------*/
/*               Initialization                                       */
/*--------------------------------------------------------------------*/

GSList *initFileTypeGroups(void)
{
  FileTypeGroups = g_slist_alloc();

  xmlFileTypes.extensions = XMLSuffixes;
  xmlFileTypes.len = 3;

  asciiFileTypes.extensions = ASCIISuffixes;
  asciiFileTypes.len = 1;


  binaryFileTypes.extensions = BinarySuffixes;
  binaryFileTypes.len = 1;

  FileTypeGroups->data = (void *) &xmlFileTypes;
  g_slist_append(FileTypeGroups, &asciiFileTypes);

  g_slist_append(FileTypeGroups, &binaryFileTypes);

  return (FileTypeGroups);
}

/*----------------------------------------------------------------------
  Initialize and populate in an InputDescription:  work out the
    mode of the data, the fully expanded name of the file, the
    base name, the name of its directory, ...
----------------------------------------------------------------------*/

InputDescription *fileset_generate(const gchar * fileName, DataMode guess,
                                   ggobid * gg)
{
  InputDescription *desc = (InputDescription *)
      calloc(1, sizeof(InputDescription));
#ifndef WIN32
  struct stat buf;
#else
  gint ft = 0;
#endif
  gint i, j;
  gint numGroups;
  GSList *groups;

  if (FileTypeGroups == NULL)
    initFileTypeGroups();

  groups = FileTypeGroups;

#ifdef SUPPORT_PLUGINS
  if (guess == unknown_data && sessionOptions->data_type) {
    GList *els = sessionOptions->info->inputPlugins;
    if (els) {
      gint i, n;
      n = g_list_length(els);
      for (i = 0; i < n; i++) {
        GGobiInputPluginInfo *info;
        info = GTK_GGOBI_INPUT_PLUGIN_INFO(g_list_nth_data(els, i));
        if (info->modeName &&
            strcmp(info->modeName, sessionOptions->data_type) == 0) {
          InputGetDescription f;
          f = (InputGetDescription) getPluginSymbol(info->getDescription,
                                                    &GTK_GGOBI_PLUGIN_DETAILS(info));

          if (f) {
            InputDescription *desc;
            desc = f(fileName, sessionOptions->data_type, gg, GTK_GGOBI_GENERAL_PLUGIN_INFO(info));
            if (desc)
              return (desc);
          }
        }
      }
    }
  }
#endif


#ifndef WIN32
  if (stat(fileName, &buf) != 0) {
#else
  if (access(fileName, ft) != 0) {
#endif
    if (isURL(fileName)) {
      desc->mode = url_data;
      desc->fileName = g_strdup(fileName);
      completeFileDesc(fileName, desc);
      return (desc);
    }

    /* Ok, so the user didn't give a full name and wants us to guess.  
       So we will look through the collections of format types and within
       each of these search through the different extensions for that type.
       If such a file exists, we assume it is of that format.
     */
    numGroups = g_slist_length(groups);
    if (guess == unknown_data) {
      for (i = 0; i < numGroups; i++) {
        gchar buf[1000];
        ExtensionList *group;
        group = (ExtensionList *) g_slist_nth_data(groups, i);
        for (j = 0; j < group->len; j++) {
          if (group->extensions[j] && group->extensions[j][0])
            sprintf(buf, "%s.%s", fileName, group->extensions[j]);
          else
            sprintf(buf, "%s", fileName);

          if (check_file_exists(buf)) {
            guess = group->mode;
            desc->mode = guess;
            desc->fileName = g_strdup(buf);
            desc->baseName = g_strdup(fileName);
            desc->givenExtension = g_strdup(group->extensions[j]);
            break;
          }
        }
        if (guess != unknown_data)
          break;
      }
    } else {
      /* Attempt to find a file of this type. */
      /*     guess = unknown_data; */
      for (i = 0; i < numGroups; i++) {
        ExtensionList *group;
        group = (ExtensionList *) g_slist_nth_data(groups, i);
        if (group->mode != guess)
          continue;

        for (j = 0; j < group->len; j++) {
          gchar buf[1000];
          if (endsWith(fileName, group->extensions[j])) {
            g_printerr("%s does not exist!\n", fileName);
            return (NULL);
          }
          if (group->extensions[j] && group->extensions[j][0])
            sprintf(buf, "%s.%s", fileName, group->extensions[j]);
          else
            sprintf(buf, "%s", fileName);
          if (check_file_exists(buf)) {
            DataMode ok = verifyDataMode(buf, group->mode, desc);
            if (ok == guess && ok != unknown_data) {
              desc->fileName = g_strdup(buf);
              desc->baseName = g_strdup(fileName);
              desc->givenExtension = g_strdup(group->extensions[j]);
              desc->mode = guess;
              guess = group->mode;
              break;
            }
          }
        }
        if (desc->fileName)
          break;
      }
    }

    if (desc->fileName == NULL) {
      guess = unknown_data;
    }

  } else {
    desc->fileName = g_strdup(fileName);
    desc->mode = guess;
    desc->mode = guess = verifyDataMode(desc->fileName, desc->mode, desc);
    if (desc->mode == unknown_data) {
      return (NULL);
    }
  }

  switch (guess) {
  case unknown_data:
    g_printerr("Cannot find a suitable file %s\n", fileName);
    return (NULL);
    break;
  default:
    break;
  }

  completeFileDesc(desc->fileName, desc);
  return (desc);
}

gint addInputSuffix(InputDescription * desc, const gchar * suffix)
{
  if (desc->extensions) {
    g_slist_append(desc->extensions, g_strdup(suffix));
  } else {
    desc->extensions = g_slist_alloc();
    desc->extensions->data = g_strdup(suffix);
  }

  return (g_slist_length(desc->extensions));
}

gint addInputFile(InputDescription * desc, const gchar * file)
{
  return (addInputSuffix(desc, file));
}

gchar *completeFileDesc(const gchar * fileName, InputDescription * desc)
{
  gint n;
  gchar *tmp = strrchr(fileName, '.');

  ExtensionList *group = getInputDescriptionGroup(desc->mode);

  if (group) {
    gint i;
    const gchar *ext;
    n = strlen(fileName);
    for (i = 0; i < group->len; i++) {
      ext = group->extensions[i];
      if (endsWith(fileName, ext)) {
        gint nbytes = strlen(fileName) - strlen(ext);
        desc->baseName = (gchar *) g_malloc(sizeof(gchar) * nbytes);
        g_snprintf(desc->baseName, nbytes, "%s", fileName);
        desc->givenExtension = g_strdup(ext);
        break;
      }
    }
    if (i == group->len) {

#if 1
      if (tmp) {
        desc->givenExtension = g_strdup(tmp + 1);
        n = (tmp - fileName) + 1;
        desc->baseName = g_malloc(sizeof(char) * n);
        g_snprintf(desc->baseName, n, "%s", fileName);
      } else {
        desc->baseName = g_strdup(fileName);
      }
#else
      return (NULL);
#endif
    }
  }

  /* Now compute the directory name. */
  if (desc->baseName) {
    tmp = strrchr(desc->baseName, G_DIR_SEPARATOR);
    if (tmp) {
      n = (tmp - desc->baseName) + 1;
      desc->dirName = g_malloc(sizeof(char) * n);
      g_snprintf(desc->dirName, n, "%s", desc->baseName);
    } else {
      desc->dirName = g_strdup(desc->baseName);
    }
  }
  return (tmp);
}

ExtensionList *getInputDescriptionGroup(DataMode mode)
{
  GSList *groups = FileTypeGroups;
  ExtensionList *el;
  while (groups) {
    el = (ExtensionList *) groups->data;
    if (el == NULL)
      return (NULL);

    if (el->mode == mode)
      return (el);

    groups = groups->next;
  }

  return (NULL);
}

void showInputDescription(InputDescription * desc, ggobid * gg)
{
  FILE *out = stderr;
  gint i;
  fprintf(out, "Input File Information:\n");
  fprintf(out, "\tFile name: %s  (extension: %s)\n",
          desc->fileName, desc->givenExtension);
  fprintf(out, "\tDirectory: %s\n", desc->dirName);
  fprintf(out, "\tFormat: %s (%d), verified: %s\n",
          GGOBI(getDataModeDescription) (desc->mode), desc->mode,
          desc->canVerify ? "yes" : "no");

  if (desc->extensions) {
    fprintf(out, "Auxillary files\n");
    for (i = 0; i < g_slist_length(desc->extensions); i++) {
      fprintf(out, "  %d) %s\n",
              i, (gchar *) g_slist_nth_data(desc->extensions, i));
    }
  }
  fflush(out);
}

/*--------------------------------------------------------------------*/
/*          Determining the mode of the data                          */
/*--------------------------------------------------------------------*/

gboolean isURL(const gchar * fileName)
{
  return ((strncmp(fileName, "http:", 5) == 0 ||
           strncmp(fileName, "ftp:", 4) == 0));
}

gboolean isASCIIFile(const gchar * fileName)
{
  FILE *f;
  gdouble val;
  f = fopen(fileName, "r");
  if (f == NULL)
    return (false);

  if (fscanf(f, "%lf", &val) == 0) {
    return (false);
  }

  return (true);
}

DataMode
verifyDataMode(const gchar * fileName, DataMode mode,
               InputDescription * desc)
{
  switch (mode) {
  case xml_data:
  case url_data:
    if (!isXMLFile(fileName, desc))
      mode = unknown_data;
    break;
  case ascii_data:
    if (!isASCIIFile(fileName))
      mode = unknown_data;
    break;
  default:
    mode = guessDataMode(fileName, desc);
  }

  return (mode);
}

gboolean isXMLFile(const gchar * fileName, InputDescription * desc)
{
  FILE *f;
  gint c;

  gchar *tmp = strrchr(fileName, '.');
  if (tmp && (strcmp(tmp, ".xmlz") == 0 || strcmp(tmp, ".gz") == 0)) {
    desc->canVerify = false;
    return (true);
  }


  f = fopen(fileName, "r");
  if (f == NULL)
    return (false);

  desc->canVerify = true;
  while ((c = getc(f)) != EOF) {
    if (c == ' ' || c == '\t' || c == '\n')
      continue;
    if (c == '<') {
      gchar buf[10];
      fgets(buf, 5, f);
      fclose(f);
      if (strcmp(buf, "?xml") == 0) {
        return (true);
      } else
        return (false);
    }
  }

  return (false);
}


DataMode guessDataMode(const gchar * fileName, InputDescription * desc)
{
  FILE *f;

  f = fopen(fileName, "r");
  if (f == NULL)
    return (unknown_data);

  if (isXMLFile(fileName, desc))
    return (xml_data);

  if (isASCIIFile(fileName))
    return (ascii_data);

  return (unknown_data);
}

/*--------------------------------------------------------------------*/
/*                 Utility functions                                  */
/*--------------------------------------------------------------------*/

gboolean endsWith(const gchar * str, const gchar * what)
{
  return (strcmp(str + strlen(str) - strlen(what), what) == 0);
}


#ifdef WIN32
gboolean check_file_exists(const gchar * fileName)
{
  gint i = 0;
  return (access(fileName, i) == 0);
}
#else
gboolean check_file_exists(const gchar * fileName)
{
  struct stat buf;
  return (stat(fileName, &buf) == 0);
}
#endif
