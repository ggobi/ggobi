#include "jvm.h"
#include "ggobi.h"
#include "ggobi_ggobi.h"

#include "GGobiAPI.h"


ggobid *getGGobiInst(JNIEnv *env, jobject obj);


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

ggobid *
getGGobiInst(JNIEnv *env, jobject obj)
{
    ggobid *gg = NULL;
    jfieldID fid;
    jdouble val;

    fid = JVMENV GetFieldID(env, JVMENV GetObjectClass(env, obj), "address", "D");
    val = JVMENV GetDoubleField(env, obj, fid);
    gg = (ggobid *) ((long) ((double) val));
    return(gg);
}
