#ifndef FILE_IO_H
#define FILE_IO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
 gchar *fileName;        /* the name of the file to read, fully expanded */
 gchar *baseName;        /* With the extension removed. */
 gchar *givenExtension;  /* the extension of the file to be read, computed when processing the file name. */
 gchar *dirName;         /* The name of the directory in which the file is located, useful for relative URIs. */
 DataMode mode;         /* The mode of the file. */
 gboolean canVerify;    /* A boolean indicating whether the format was verified, which is not possible
                            e.g. when reading a zipped xml file, 
                          */
  GSList *extensions;     /* */
} InputDescription;

typedef struct {
  DataMode mode;            /*  The mode or format style to which the extensions apply. */

  char **extensions;     /* A list of extensions to append to a file name when looking for a file 
                           of this format.
                              e.g.  xml, xml.gz, xmlz
                                    dat
                                    bin
                         */

  int len;     /* Number of elements in the extensions array. */
} ExtensionList;



GSList *initFileTypeGroups(void);
DataMode verifyDataMode(const char *fileName, DataMode mode, InputDescription *desc);
DataMode guessDataMode(const char *fileName, InputDescription *desc);
gboolean isXMLFile(const char *fileName, InputDescription *desc);
gboolean isASCIIFile(const char *fileName);
gchar *computeExtension(const char *fileName);

gchar *completeFileDesc(const char *fileName, InputDescription *desc);

ExtensionList *getInputDescriptionGroup(DataMode mode);

gboolean check_file_exists(const char *fileName);

gboolean endsWith(const char *str, const char *what);
InputDescription* fileset_generate(const char *fileName, DataMode guess);


gchar * findAssociatedFile(InputDescription *desc, const gchar * const *suffixes, int numSuffixes, gint *which, gboolean isError);
int addInputSuffix(InputDescription *desc, const gchar *suffix);
int addInputFile(InputDescription *desc, const gchar *file);

#ifdef __cplusplus
}
#endif

#endif

