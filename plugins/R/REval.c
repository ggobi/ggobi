#include "RSCommon.h"
#include "REval.h"

typedef struct {
    USER_OBJECT_ expression;
    USER_OBJECT_ val;
} ProtectedEvalData;

void
protectedEval(void *d)
{
    ProtectedEvalData *data = (ProtectedEvalData *)d;

    data->val = eval(data->expression, R_GlobalEnv); 
    PROTECT(data->val);
}

USER_OBJECT_
tryEval(USER_OBJECT_ e, int *ErrorOccurred)
{
 Rboolean ok;
 ProtectedEvalData data;

 data.expression = e;
 data.val = NULL;

 ok = R_ToplevelExec(protectedEval, &data);
 if(ErrorOccurred) {
     *ErrorOccurred = (ok == FALSE);
 }
 if(ok == FALSE)
     data.val = NULL;
 else
     UNPROTECT(1);

 return(data.val);
}
