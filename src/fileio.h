/* fileio.h */
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
*/

#ifndef FILE_IO_H
#define FILE_IO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GGobiInputPluginInfo GGobiInputPluginInfo;
typedef struct _GGobiPluginInfo GGobiPluginInfo;
typedef struct _ggobid ggobid;
typedef struct _InputDescription InputDescription;

typedef GSList* (*InputReader)(InputDescription *desc, ggobid *gg, GGobiPluginInfo *);
typedef gboolean (*InputProbe)(const char * const input, ggobid *gg, GGobiPluginInfo *);

typedef InputDescription* (*InputGetDescription)(const char * const fileName, const char * const input, ggobid *gg, GGobiPluginInfo*);


struct _InputDescription {
 gchar *fileName;       /* the name of the file to read, fully expanded */
 gchar *baseName;       /* With the extension removed. */
 gchar *givenExtension; /* the extension of the file to be read,
                           computed when processing the file name. */
 gchar *dirName;        /* The name of the directory in which the file is
                           located, useful for relative URIs. */
 DataMode mode;         /* The mode of the file. */
 gboolean canVerify;    /* A boolean indicating whether the format was
                           verified, which is not possible
                           e.g. when reading a zipped xml file, 
                         */
 GSList *extensions;     /* a collection of file extension names and modes. */

 void *userData;
 InputReader desc_read_input;
};

typedef struct {
  DataMode mode;         /* The mode or format style to which the extensions
                            apply. */

  char **extensions;     /* A list of extensions to append to a file name
                            when looking for a file 
                            of this format.
                            e.g.  xml, xml.gz, xmlz
                                  dat
                                  bin
                         */

  gint len;     /* Number of elements in the extensions array. */
} ExtensionList;


gboolean isURL(const gchar *fileName);
gboolean check_file_exists(const gchar *fileName);
GSList *initFileTypeGroups(void);
DataMode verifyDataMode(const gchar *fileName, DataMode mode, InputDescription *desc);
DataMode guessDataMode(const gchar *fileName, InputDescription *desc);
gboolean isXMLFile(const gchar * fileName, ggobid *gg, GGobiPluginInfo *info);
gboolean isCSVFile(const gchar * fileName, ggobid *gg, GGobiPluginInfo *info);
gboolean isASCIIFile(const gchar * fileName, ggobid *gg, GGobiPluginInfo *plugin);
gboolean endsWith(const gchar *str, const gchar *what);
ExtensionList *getInputDescriptionGroup(DataMode mode);

/*
gchar *computeExtension(const gchar *fileName);
*/

gchar *completeFileDesc(const gchar *fileName, InputDescription *desc);


//gchar *findAssociatedFile(InputDescription *desc, const gchar * const *suffixes, gint numSuffixes, gint *which, gboolean isError);
gint addInputSuffix(InputDescription *desc, const gchar *suffix);
gint addInputFile(InputDescription *desc, const gchar *file);

extern gboolean canRead(const char * const fileName);

#ifdef __cplusplus
}
#endif

#endif

