#ifndef GGOBI_JVM_H
#define GGOBI_JVM_H

#include <jni.h>
#include "plugin.h"

struct _JavaRunTimeData {
    jclass klass;

    jobject instance;

    jmethodID getRecord;
};


typedef enum {GGobi_LOAD, GGobi_UNLOAD, GGobi_OPEN, GGobi_CLOSE, 
              GGobi_UPDATE_DISPLAY, GGobi_NUM_PLUGIN_METHODS} GGobiPluginMethodIDs;
typedef struct {
    jobject obj;
    jmethodID mids[GGobi_NUM_PLUGIN_METHODS];
} JavaPluginInstance;


#define JVMENV (*env)->


#endif
