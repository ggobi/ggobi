#include <windows.h>
#include <oaidl.h>
#include <oleauto.h>

#include <stdio.h>

extern "C" { 
#include "ggobi.h"
#include "GGobiAPI.h"
#include "plugin.h"

InputDescription *ExcelDataDescription(const char * const fileName, const char * const modeName, ggobid *gg, GGobiPluginInfo *info);
gboolean onLoad(gboolean initializing, GGobiPluginInfo *plugin);
}

extern void COMError(HRESULT hr);
extern void GetScodeString(HRESULT hr, LPTSTR buf, int bufSize);

BSTR AsBstr(char *str);

IDispatch *getWorkbooks(IDispatch *);
HRESULT getProperty(IDispatch *, BSTR name, VARIANT *v);
IDispatch *call(IDispatch *iface, BSTR name, VARIANT *args, int numArgs);

int createDataset(VARIANT *var, ggobid *gg);

gboolean readData(InputDescription *desc, ggobid *gg, GGobiPluginInfo *plugin);

gboolean
onLoad(gboolean initializing, GGobiPluginInfo *plugin)
{
 HRESULT hr;
 hr = CoInitialize(NULL);
 return(!FAILED(hr) ? true : false);
}

//typedef char gchar;

void
Error(char *str)
{
  fprintf(stderr, "%s\n", str);
  exit(1);
}

/* datad * */
void
readDataFile(gchar *fileName, ggobid *gg)
{
  HRESULT hr;
  CLSID classID;

  IDispatch *iface;

  hr = CLSIDFromString(L"Excel.Application", &classID);

  if(FAILED(hr)) {
    Error("Can't get Excel's class ID");
  }

  hr = CoCreateInstance(classID, NULL, CLSCTX_SERVER, IID_IDispatch, (void **) &iface);

  if(FAILED(hr)) {
    Error("Can't create Excel instance");
  }

  IDispatch *books = getWorkbooks(iface);

  VARIANT v;
  VariantInit(&v);
  V_VT(&v) = VT_BSTR;
  V_BSTR(&v) = AsBstr("D:\\duncan\\quakes.csv");
  //  V_BSTR(&v) = L"D:\\duncan\\quakes.csv";

  IDispatch *sheet, *cells, *s;

  s = call(books, L"Open", &v, 1);
  getProperty(s, L"ActiveSheet", &v);
    sheet = V_DISPATCH(&v);  
  getProperty(sheet, L"UsedRange", &v);
    cells = V_DISPATCH(&v);
  getProperty(cells, L"Value", &v);
  fprintf(stderr, "Vaues: %d, array? %d\n", V_VT(&v), (int) (V_ISARRAY(&v) ? 1 : 0));
  createDataset(&v, gg);
 
  cells->Release();
  sheet->Release();
  s->Release();
  books->Release();
  iface->Release();
}

IDispatch *
getWorkbooks(IDispatch *iface)
{
  VARIANT v;
  HRESULT hr;
  hr = getProperty(iface, L"Workbooks", &v);
  IDispatch *books;
  books = V_DISPATCH(&v);
  books->AddRef();
  return(books);
}

HRESULT
getProperty(IDispatch *iface, BSTR name, VARIANT *v)
{
  VariantInit(v);
  DISPID mid;
  HRESULT hr;
  DISPPARAMS params = {NULL, NULL, 0, 0};
  hr = iface->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &mid);
  if(FAILED(hr))
    Error("Can't get id of name");

  hr = iface->Invoke(mid, IID_NULL, LOCALE_USER_DEFAULT, INVOKE_PROPERTYGET, &params, v, NULL, NULL);
  if(FAILED(hr))
    Error("Can't get property");

  return(hr);
}


IDispatch *
call(IDispatch *iface, BSTR name, VARIANT *args, int numArgs)
{
  VARIANT v;
  VariantInit(&v);
  DISPID mid;
  HRESULT hr;
  DISPPARAMS params = {NULL, NULL, 0, 0};

  hr = iface->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &mid);
  if(FAILED(hr))
    Error("Can't get id of name");

 params.rgvarg = args;
 params.cArgs = numArgs;

 EXCEPINFO exceptionInfo;
 UINT nargErr = 1000;
 memset(&exceptionInfo, 0, sizeof(exceptionInfo));

 hr = iface->Invoke(mid, IID_NULL, LOCALE_USER_DEFAULT, INVOKE_FUNC, &params, &v, &exceptionInfo, &nargErr);
 if(FAILED(hr)) {
    fprintf(stderr, "Error message %d %d\n", (exceptionInfo.bstrDescription || exceptionInfo.bstrSource ?  1 : 0), (int) nargErr);
    COMError(hr);
    Error("Can't call method");
 }

 IDispatch *ans = V_DISPATCH(&v);
 return(ans);
}

InputDescription *
ExcelDataDescription(const char * const fileName, const char * const modeName, 
                             ggobid *gg, GGobiPluginInfo *info)
{
  InputDescription *desc;
  desc = (InputDescription*) g_malloc(sizeof(InputDescription));
  memset(desc, '\0', sizeof(InputDescription));

  desc->fileName = g_strdup("");
  //  desc->name = g_strdup("<Excel>");
  desc->mode = unknown_data;
  desc->desc_read_input = readData;

  fprintf(stderr, "In Description %s\n", fileName);fflush(stderr);
  return(desc);
}

gboolean 
readData(InputDescription *desc, ggobid *gg, GGobiPluginInfo *plugin)
{
  readDataFile("D:\\duncan\\quakes.csv", gg);
  return(true);
}

#ifdef USE_MAIN
int
main(int argc, char *argv[])
{
  CoInitialize(NULL);
  readDataFile(NULL);
  fprintf(stderr, "We're done\n");fflush(stderr);
}
#endif

double
asReal(VARIANT *v)
{
  VariantChangeType(v, v, 0, VT_R8);
  return(V_R8(v));
}

int
createDataset(VARIANT *var, ggobid *gg)
{
  SAFEARRAY *arr;
  VARIANT value;

  if(V_ISBYREF(var))
    arr = *V_ARRAYREF(var);
  else
    arr = V_ARRAY(var);

  long lb, ub;
  long i, j, ctr;
  UINT numDim = SafeArrayGetDim(arr);
  fprintf(stderr, "Num. dimension %d\n", (int) numDim);
  long dim[2][2];

  for(i = 0; i < numDim; i++) {
    SafeArrayGetLBound(arr, i+1, &dim[i][0]);
    SafeArrayGetUBound(arr, i+1, &dim[i][1]);
    fprintf(stderr, "Dimension %d: %ld %ld\n", i, dim[i][0], dim[i][1]);
  }

  datad *d = NULL;
  d = datad_new(d, gg);
  d->nrows = dim[0][1] - dim[0][0]; // + 1;
  d->ncols = dim[1][1] - dim[1][0] + 1;

  fprintf(stderr, "Dimensions %d x %d\n", d->nrows, d->ncols);

  long indices[2];
  gdouble *vals = (gdouble *)g_malloc(sizeof(double) * d->nrows);
  for(j = dim[1][0];  j <= dim[1][1]; j++) {
    for(ctr = 0, i = dim[0][0] + 1; i <= dim[0][1]; i++, ctr++) {
      indices[0] =i;
      indices[1] =j;
      SafeArrayGetElement(arr, indices, &value);
      vals[ctr] = asReal(&value);
    }
    fprintf(stderr, "Variable....\n");
    int varId = GGOBI(addVariable)(vals, d->nrows, g_strdup_printf("Var %d", j+1), true, d, gg);
    fprintf(stderr, "Variable %d\n", varId);
  }
  g_free(vals);

 return(0);
}



BSTR
AsBstr(char *str)
{
  BSTR ans = NULL;
  if(!str)
    return(NULL);
  int size = strlen(str);
  int wideSize = 2 * size;
  LPOLESTR wstr = (LPWSTR) malloc(wideSize); /*XXX Should be S_alloc() */
  MultiByteToWideChar(CP_ACP, 0, str, size, wstr, wideSize);
  ans = SysAllocStringLen(wstr, size);
  free(wstr);
  return(ans);
}
