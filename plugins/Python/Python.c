#include "GGobiAPI.h"
#include "plugin.h"

#include <Python.h>

typedef struct{
	char *className;
	char *moduleName;
	PyObject *inst;
} PythonInputPluginData;




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
Py_onClose(ggobid *gg, GGobiPluginInfo *info, PluginInstance *inst)
{
  return(CallPluginMethod("onClose", gg, inst));
}

gboolean
Py_onUpdateDisplay(ggobid *gg, PluginInstance *inst)
{
  return(CallPluginMethod("onUpdateDisplay", gg, inst));
}

gboolean
CallPluginMethod(const char *name, ggobid *gg, PluginInstance *inst)
{
  PyObject *pinst;
  PyObject *pmethod, *pargs, *value;
  gboolean status = true;

#ifdef PY_GGOBI_DEBUG
  g_printerr("Calling plugin method %s\n", name);
#endif

  pinst =  (PyObject *) (inst->data);
  pmethod = PyObject_GetAttrString(pinst, (char *) name);
  if(CheckPythonError()) 
    return(false);

  pargs = NULL;
  value = PyEval_CallObject(pmethod, pargs);

  if(CheckPythonError()) 
    return(false);

  if(PyInt_Check(value))
    status = PyInt_AsLong(value);
  else {
    g_printerr("Return value from method %s was not an integer", name);
    status = false;
  }

  Py_DECREF(value);

  return(status);
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
	g_printerr("\n");
	return(true);
  }
      return(false);
}


gboolean 
PythonCreatePlugin(ggobid *gg, GGobiPluginInfo *info, PluginInstance *inst)
{
   PyObject *module, *obj, *d, *klass;
   PythonInputPluginData *data = (PythonInputPluginData *) info->data;

#ifdef PY_GGOBI_DEBUG
   g_printerr("Creating plugin %s\n", data->className);
#endif

   module = PyImport_ImportModule(data->moduleName ? data->moduleName : "__main__");
   if(CheckPythonError()) 
	   return(false);

   d = PyModule_GetDict(module);
   klass = PyMapping_GetItemString(d, data->className);
   PyClass_Check(klass);
   obj = PyInstance_New(klass, NULL, NULL);
   if(CheckPythonError())
     return(false);

   Py_INCREF(obj);
   inst->data = obj;

   return(true);
}

gboolean
Python_processPlugin(xmlNodePtr node, GGobiPluginInfo *plugin, GGobiPluginType type, 
                      GGobiPluginInfo *langPlugin, GGobiInitInfo *info)
{

      const xmlChar *tmp;
      PythonInputPluginData *data;
      GGobiPluginDetails *details;


      data = (PythonInputPluginData *)g_malloc(sizeof(PythonInputPluginData));
      memset(data, '\0', sizeof(PythonInputPluginData));
      plugin->data = data;
      details = plugin->details;

      tmp = xmlGetProp(node, "class");
      data->className = g_strdup(tmp); 
      tmp = xmlGetProp(node, "module");
      data->moduleName = g_strdup(tmp); 

#ifdef PY_GGOBI_DEBUG
      g_printerr("In Python_processPlugin %s\n", data->className);
#endif

      if(type == INPUT_PLUGIN) {
        plugin->info.i->getDescription = g_strdup("PythonGetInputDescription");

      } else {
        plugin->info.g->onCreate = g_strdup("PythonCreatePlugin");
        plugin->info.g->onClose = g_strdup("Py_onClose");
        plugin->info.g->onUpdateDisplay = g_strdup("Py_onUpdateDisplay"); 
      }

      setLanguagePluginInfo(details, "Python", info);
/*
      details->onLoad = g_strdup("PythonLoadPlugin");
      details->onUnload = g_strdup("PythonUnloadPlugin");
*/

      return(true);
}
