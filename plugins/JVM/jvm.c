#include "jvm.h"

#include "GGobiAPI.h"

static JNIEnv *std_env = NULL;
static JavaVM *jvm;

#define CLASSPATH_OPT "-Djava.class.path="

#define JVMENV (*env)->

jobject runPlugin(const char * const klass, JNIEnv *env);
gboolean initJVM(void);

char *getInputDescription(JavaInputPluginData *rt, JNIEnv *env);
jboolean getDims(int *nrow, int *ncol, JavaRunTimeData *rt, JNIEnv *env);

/**
  This is the initial entry point for the plugin that starts the
  virtual machine.
 */
gboolean loadJVM(gboolean initializing, GGobiPluginInfo *pluginInfo)
{
    return(initJVM());
} 

/**
  This starts the Virtual Machine.
 */
gboolean initJVM()
{
  jboolean status = JNI_FALSE;
  JavaVMInitArgs vm2_args;
  JavaVMInitArgs *vm_args = &vm2_args;
  JavaVMOption options[1];
  char *classpath="plugins/JVM", *ptr;
  jint res;

    vm_args = (JavaVMInitArgs *) &vm2_args;
    vm_args->version = JNI_VERSION_1_2;

    options[0].optionString = ptr = (char*) calloc(strlen(CLASSPATH_OPT) + strlen(classpath) + 1, sizeof(char));
    strcpy(ptr, CLASSPATH_OPT);
    strcat(ptr, classpath);

    vm_args->options = options;
    vm_args->nOptions = sizeof(options)/sizeof(options[0]);
    vm_args->ignoreUnrecognized = JNI_FALSE;
    if(JNI_GetDefaultJavaVMInitArgs(vm_args) != 0) {
	return(JNI_FALSE);
    }

    res = JNI_CreateJavaVM(&jvm, (void**) &std_env, (void*) vm_args);

    if(res < 0 || std_env == NULL) {
	fprintf(stderr, "Can't create JVM\n");
        return(JNI_FALSE);
    }

    status = JNI_TRUE;
    /* runPlugin("ggobi/ggobi", std_env) != NULL; */
    return(status);
}

jobject
runPlugin(const char * const klass, JNIEnv *env)
{

 jclass cls;
 jmethodID id;
 jobject obj;

   cls = (*env)->FindClass(env, klass); 
   if(!cls) {
       fprintf(stderr, "Cannot find class %s\n", klass);
       exit(3);
   }
   id = (*env)->GetMethodID(env, cls, "<init>", "()V");
   if(!id) {
       fprintf(stderr, "Cannot find method in class %s\n", klass);
       exit(3);
   }
   obj = (*env)->NewObject(env, cls, id);
   (*env)->DeleteLocalRef(env, cls);

   return(obj);
}


gboolean 
JavaReadInput(InputDescription *desc, ggobid *gg, GGobiInputPluginInfo *plugin)
{
    datad *gdata;
    JavaRunTimeData *rt = ((JavaInputPluginData *) plugin->data)->runTime;
    int i, j;
    int nrow, ncol;

    jmethodID mid;
    jobjectArray jnames;
    jstring jname;
    JNIEnv *env = std_env;

/* Need to figure out who is actually calling this and if it supplies the init. */
gboolean init = true;

    if(getDims(&nrow, &ncol, rt, env) == JNI_FALSE)
	return(false);

    gdata = datad_create(nrow, ncol, gg);

    gdata->name = g_strdup(desc->fileName);

    for(j = 0; j < ncol; j++) {
	const char *tmp;
        jboolean isCopy;
        mid = JVMENV GetMethodID(env, rt->klass, "getVariableNames", "()[Ljava/lang/String;");
        jnames = JVMENV CallObjectMethod(env, rt->instance, mid);
        jname = JVMENV GetObjectArrayElement(env, jnames, j);
        tmp =  JVMENV GetStringUTFChars(env, jname, &isCopy);
	GGOBI(setVariableName)(j, g_strdup(tmp), false, gdata, gg);
	JVMENV ReleaseStringUTFChars(env, jname, tmp);
    }

    mid = JVMENV GetMethodID(env, rt->klass, "getRecord", "(I)[D");
    for(i = 0 ; i < nrow; i++) {
        jdouble *els;
        jboolean isCopy;
        jdoubleArray obj = JVMENV CallObjectMethod(env, rt->instance, mid, (jint) i);
        els = JVMENV GetDoubleArrayElements(env, obj, &isCopy);
        for(j = 0; j < ncol; j++)
	    gdata->raw.vals[i][j] = els[j];
        JVMENV ReleaseDoubleArrayElements(env, obj, els, JNI_ABORT);
    }

    start_ggobi(gg, true, init);

    return(true);
}

InputDescription *
JavaGetInputDescription(const char * const fileName, const char * const modeName,
                         ggobid *gg, GGobiInputPluginInfo *info)
{
    JavaInputPluginData *data = (JavaInputPluginData *) info->data;
    InputDescription *desc;
    JNIEnv *env = std_env;

    if(data->runTime == NULL) {
        jclass klass;
        jmethodID cid;
        jstring jfn = NULL, jmode = NULL;

	data->runTime = g_malloc(sizeof(JavaRunTimeData));
        memset(data->runTime, '\0',sizeof(JavaRunTimeData));

        klass = JVMENV FindClass(env, data->className);
        cid = JVMENV GetMethodID(env, klass, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
        if(fileName)
	    jfn = JVMENV NewStringUTF(env, fileName);
        if(modeName)
	    jmode = JVMENV NewStringUTF(env, modeName);
        data->runTime->instance = JVMENV NewObject(env, klass, cid, jfn, jmode);
	data->runTime->klass = klass;
    }

    desc = (InputDescription *) g_malloc(sizeof(InputDescription));
    desc->fileName = getInputDescription(data, env);
#ifdef JAVA_DEBUG
    fprintf(stderr, "Input source fileName: %s\n", desc->fileName);fflush(stderr);
#endif
    desc->mode = unknown_data;
    desc->desc_read_input = JavaReadInput;
    return(desc);
}

char *
getInputDescription(JavaInputPluginData *data, JNIEnv *env)
{

    jmethodID mid;
    jobject val;
    char *ptr;
    jboolean isCopy;
    jstring jstr;
    const char *tmp;

    mid = JVMENV GetMethodID(env, data->runTime->klass, "getSourceDescription", "(Z)Ljava/lang/String;");
    jstr = (jstring) JVMENV CallObjectMethod(env, data->runTime->instance, mid);
    
    tmp =  JVMENV GetStringUTFChars(env, jstr, &isCopy);
    
    ptr = g_strdup(tmp);
    JVMENV ReleaseStringUTFChars(env, jstr, tmp);
    return(ptr);
}


jboolean
getDims(int *nrow, int *ncol, JavaRunTimeData *rt, JNIEnv *env)
{
    jmethodID mid;

    mid = JVMENV GetMethodID(env, rt->klass, "getNumRecords", "()I");
    *nrow = JVMENV CallIntMethod(env, rt->instance, mid);

    mid = JVMENV GetMethodID(env, rt->klass, "getNumVariables", "()I");
    *ncol = JVMENV CallIntMethod(env, rt->instance, mid);

    return(JNI_TRUE);
}




gboolean
JavaLoadPlugin(gboolean initializing, GGobiPluginInfo *plugin)
{
    jboolean ans = JNI_TRUE;
    jmethodID mid;
    jclass klass;
    JNIEnv *env = std_env;

    klass = JVMENV FindClass(env, ((JavaInputPluginData*)plugin->data)->className);
    mid = JVMENV GetStaticMethodID(env, klass, "onLoad", "()Z");
    if(mid)
	ans = JVMENV CallStaticBooleanMethod(env, klass, mid);

    return(ans);
}

gboolean
JavaUnloadPlugin(gboolean initializing, GGobiPluginInfo *plugin)
{
    jboolean ans = JNI_TRUE;
    jmethodID mid;
    jclass klass;
    JNIEnv *env = std_env;

    klass = JVMENV FindClass(env, ((JavaInputPluginData*)plugin->data)->className);
    mid = JVMENV GetStaticMethodID(env, klass, "onUnload", "()Z");
    if(mid)
	ans = JVMENV CallStaticBooleanMethod(env, klass, mid);

    return(ans);
}


gboolean 
JavaCreatePlugin(ggobid *gg, GGobiPluginInfo *info, PluginInstance *inst)
{
    jboolean ans = JNI_TRUE;
    jclass klass;
    JavaInputPluginData *data = (JavaInputPluginData *) info->data;
    jmethodID cid;
    jobject obj;
    JNIEnv *env = std_env;

    data->runTime = g_malloc(sizeof(JavaRunTimeData));
    memset(data->runTime, '\0',sizeof(JavaRunTimeData));

    klass = JVMENV FindClass(env, data->className);

    cid = JVMENV GetMethodID(env, klass, "<init>","()V");
    obj = JVMENV NewObject(env, klass, cid);

    if(obj) {
        JavaPluginInstance *tmp;
        inst->data = tmp = g_malloc(sizeof(JavaPluginInstance));
        obj = JVMENV NewGlobalRef(env, obj);
	tmp->obj = obj;

        tmp->mids[GGobi_CLOSE] = JVMENV GetMethodID(env, klass, "onClose", "()Z");
        tmp->mids[GGobi_UPDATE_DISPLAY] = JVMENV GetMethodID(env, klass, "onUpdateDisplay", "()Z");
        if(tmp->mids[GGobi_UPDATE_DISPLAY]) {
            /* Register for events of updating the display menu. */           
	}
    } else {
        data->runTime = NULL;
	g_free(data->runTime);
    }

    return(ans);
}

gboolean 
JavaDestroyPlugin(ggobid *gg, GGobiPluginInfo *info, PluginInstance *inst)
{
    jboolean ans = JNI_TRUE;
    jobject obj;
    JavaPluginInstance *d = inst->data;
    JNIEnv *env = std_env;
    
    obj = d->obj;

    if(d->mids[GGobi_CLOSE]) {
	ans = JVMENV CallBooleanMethod(env, obj, d->mids[GGobi_CLOSE]);
    }

    JVMENV DeleteGlobalRef(env, obj);
    g_free(d);

    return(ans);
}


gboolean 
JavaUpdateDisplayMenu(ggobid *gg, PluginInstance *inst)
{

   JavaPluginInstance *d = inst->data;
   jmethodID mid;
   JNIEnv *env = std_env;
   jboolean ans = JNI_TRUE;

   if((mid = d->mids[GGobi_UPDATE_DISPLAY]) == NULL) {
       return(JNI_FALSE);
   } 

   ans = JVMENV CallBooleanMethod(env, d->obj, mid);
   return(ans);
}
