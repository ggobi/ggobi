#include "GGobiAPI.h"
#include "plugin.h"

#include <Python.h>



gboolean CheckPythonError();
gboolean initPython(GGobiPluginInfo *info);

/**
  This is the initial entry point for the plugin that starts the
  Python interpreter.
  @see initPython()
 */
gboolean 
loadPython(gboolean initializing, GGobiPluginInfo *pluginInfo)
{
    return(initPython(pluginInfo));
} 

gboolean 
initPython(GGobiPluginInfo *info)
{
  const char * pythonpath = NULL;
  char *buf;

   pythonpath = getCommandLineArgValue("pythonpath");   
   if(!pythonpath && info->details->namedArgs) {
      pythonpath = (char *) g_hash_table_lookup(info->details->namedArgs, "pythonpath");  
   }
   if(pythonpath && pythonpath[0]) {
     buf = (char *) malloc(sizeof(char) * (strlen(pythonpath) + 12));
     sprintf(buf, "PYTHONPATH=%s", pythonpath);
     putenv(buf);
   }

   Py_Initialize();    
   CheckPythonError();

   PyImport_ImportModule("GGobiPlugin");
   CheckPythonError();

   return(true);
}

gboolean
CheckPythonError()
{
  if(PyErr_Occurred()) {
      int clear = 1;
      char buf[1000];
      PyObject *exception, *v, *tb;
      PyErr_Fetch(&exception, &v, &tb);
      PyErr_NormalizeException(&exception, &v, &tb);

        /* Handle the old-style exceptions which are just simple strings. */
      if(PyString_Check(exception)) {
	 sprintf(buf,"%s", PyString_AsString(exception));
      } else {
	  /* Otherwise, we have a real exception and we can get its
             error message via __str__()
             Are all exceptions guaranteed to have this method?
           */
	PyObject *m = PyObject_GetAttrString(v, "__str__");
	PyObject *args = PyTuple_New(0);
        PyObject *str = PyEval_CallObject(m, args);

	if(str && PyString_Check(str)) {
	    sprintf(buf, "%s", PyString_AsString(str));
	} else
	    sprintf(buf, "%s", "A Python Exception Occurred.");

      }
	g_printerr(buf);
	return(true);
  }
      return(false);
}

