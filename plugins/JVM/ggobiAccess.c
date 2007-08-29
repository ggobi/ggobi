#include "jvm.h"
#include "session.h"

#include "GGobiAPI.h"

void  *getAddressInst(JNIEnv *env, jobject obj);
ggobid *getGGobiInst(JNIEnv *env, jobject obj);
jobject createJavaDatad(JNIEnv *env, GGobiData *d);

jint
Java_ggobi_ggobi_getNumDatasets(JNIEnv *env, jobject jgg)
{
  ggobid *gg;
  gg = getGGobiInst(env, jgg);


  return((jint) g_slist_length(gg->d));
}

jstring
Java_ggobi_ggobi_getDescription(JNIEnv *env, jobject jgg)
{
  ggobid *gg;
  gchar *tmp;
  jstring str;
  gg = getGGobiInst(env, jgg);

  tmp = GGobi_getDescription(gg);
  str = JVMENV NewStringUTF(env, tmp);

  return(str);
}



jstring
Java_ggobi_datad_getName(JNIEnv *env, jobject obj)
{
  jstring str = NULL;
  GGobiData *d = (GGobiData*) getAddressInst(env, obj);
  if(!d) {
      return(NULL);
  }

  if(d->name)
      str = JVMENV NewStringUTF(env, d->name);
  return(str);
}

jobject
Java_ggobi_ggobi_getDataset__I(JNIEnv *env, jobject jgg, jint which)
{
    GGobiData *d;
    ggobid *gg;
    gg = getGGobiInst(env, jgg);
    d = (GGobiData*) g_slist_nth_data(gg->d, which);
    return(createJavaDatad(env, d));
}

jint
Java_ggobi_datad_getNumRecords(JNIEnv *env, jobject jdata)
{
    GGobiData *d;
    d = (GGobiData*) getAddressInst(env, jdata);
    return(d->nrows);
}


ggobid *
getGGobiInst(JNIEnv *env, jobject obj)
{
    ggobid *gg = NULL;
    gg = (ggobid *)getAddressInst(env, obj);
    return(gg);
}


void* 
getAddressInst(JNIEnv *env, jobject obj)
{
    jfieldID fid;
    jdouble val;

    fid = JVMENV GetFieldID(env, JVMENV GetObjectClass(env, obj), "address", "D");
    val = JVMENV GetDoubleField(env, obj, fid);
    return((void*) ((long) ((double) val)));
}

jobject
createJavaDatad(JNIEnv *env, GGobiData *d)
{
    jmethodID id;
    jobject obj;
    jclass klass;
    klass = JVMENV FindClass(env, "ggobi/datad");
    if(klass == NULL) {
	fprintf(stderr, "Can't get ggobi.datad class\n");fflush(stderr); 
        if(JVMENV ExceptionOccurred(env)) {
	    JVMENV ExceptionDescribe(env);
	}
        return(NULL);
    }
    id = JVMENV GetMethodID(env, klass, "<init>", "(D)V");
    obj = JVMENV NewObject(env, klass, id, (double) (long) d);
    return(obj);
}
