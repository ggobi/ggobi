#include <windows.h>
#include <oaidl.h>
#include <oleauto.h>

#include <stdio.h>

extern "C" { 
#include "ggobi.h"
  //#include "externs.h"
#include "GGobiAPI.h"
#include "plugin.h"

displayd*  datad_init (datad *, ggobid *, gboolean);

extern const gchar **getDefaultRowNamesPtr();

InputDescription *ExcelDataDescription(const char * const fileName, const char * const modeName, ggobid *gg, GGobiPluginInfo *info);
gboolean onLoad(gboolean initializing, GGobiPluginInfo *plugin);
}

#define MY_V_DISPATCH(v)  V_DISPATCH(v)

void readDataFile(gchar *fileName, InputDescription *desc, ggobid *gg);

extern void COMError(HRESULT hr);
extern void GetScodeString(HRESULT hr, LPTSTR buf, int bufSize);

void Error(char *str);
BSTR AsBstr(char *str);
char *FromBstr(BSTR str);


IDispatch *getWorkbooks(IDispatch *);
HRESULT getProperty(IDispatch *, BSTR name, VARIANT *v);
IDispatch *call(IDispatch *iface, BSTR name, VARIANT *args, int numArgs);

datad *createDataset(VARIANT *var, ggobid *gg);

gboolean readData(InputDescription *desc, ggobid *gg, GGobiPluginInfo *plugin);

datad *loadSheet(IDispatch *sheet, ggobid *gg, gchar *fileName);


gboolean
onLoad(gboolean initializing, GGobiPluginInfo *plugin)
{
 HRESULT hr;
 hr = CoInitialize(NULL);
 return(!FAILED(hr) ? true : false);
}


InputDescription *
ExcelDataDescription(const char * const fileName, const char * const modeName, 
                             ggobid *gg, GGobiPluginInfo *info)
{
  InputDescription *desc;
 
  if(!fileName || !fileName[0]) {
    fprintf(stderr, "No file name specified for the ExcelReader plugin to read.\n"); fflush(stderr);
    //    return(NULL);
  }

  desc = (InputDescription*) g_malloc(sizeof(InputDescription));
  memset(desc, '\0', sizeof(InputDescription));

  desc->fileName = g_strdup(fileName ? fileName : "");

  desc->mode = unknown_data;
  desc->desc_read_input = readData;

  return(desc);
}

gboolean 
readData(InputDescription *desc, ggobid *gg, GGobiPluginInfo *plugin)
{
  readDataFile(desc->fileName, desc, gg);
  return(true);
}

void
releaseVariants(VARIANT *vars, int n, gboolean release)
{
  for(int i = 0; i < n; i++) {
    if(release && V_VT(&vars[i]) == VT_DISPATCH) {
      V_DISPATCH(&vars[i])->Release();
    }
    VariantClear(&vars[i]);
  }
}

/* datad * */
void
readDataFile(gchar *fileName, InputDescription *desc, ggobid *gg)
{
  HRESULT hr;
  CLSID classID;

  IDispatch *iface;

  hr = CLSIDFromString(L"Excel.Application", &classID);
  
  if(FAILED(hr)) {
    Error("Can't get Excel's class ID");
  }

  if(fileName && fileName[0])
    hr = CoCreateInstance(classID, NULL, CLSCTX_SERVER, IID_IDispatch, (void **) &iface);
  else {
    fprintf(stderr, "Connecting to existing Excel application instance\n");fflush(stderr);
    hr = GetActiveObject(classID, NULL, (IUnknown **)&iface);
  }

  if(FAILED(hr)) {
    COMError(hr);
    Error("Can't create Excel instance");
  }

  IDispatch *books = getWorkbooks(iface);

  IDispatch *sheet, *cells, *s;

  VARIANT *v, vars[5];
  for(int i = 0; i < sizeof(vars)/sizeof(vars[0]); i++) {
    VariantInit(vars+i);
  }

  if(!fileName || !fileName[0])
    fileName = "D:\\duncan\\quakes.csv";

  v = &vars[0];
  V_VT(v) = VT_BSTR;
  V_BSTR(v) = AsBstr(fileName);   // XXX doesn't work V_BSTR(&v) = L"D:\\duncan\\quakes.csv";


  s = call(books, L"Open", v, 1);

#if 0
  getProperty(s, L"ActiveSheet", v = &vars[2]);
    sheet = V_DISPATCH(v);  

  getProperty(sheet, L"UsedRange", v = &vars[3]);
    cells = V_DISPATCH(v);
  getProperty(cells, L"Value", v = &vars[4]);

  datad *d = createDataset(v, gg);
  d->name = g_strdup(fileName);

  releaseVariants(vars, sizeof(vars)/sizeof(vars[0]), 0);

  call(s, L"Close", NULL, 0);
  fprintf(stderr, "Closed() the sheet\n"); fflush(stderr);

  cells->Release();
  sheet->Release();
  s->Release();
  releaseVariants(vars, sizeof(vars)/sizeof(vars[0]), true);
#else

  getProperty(s, L"Sheets", v = &vars[2]);
  IDispatch *sheets = V_DISPATCH(v); 
  sheets->AddRef();
  getProperty(sheets, L"Count", v = &vars[3]);
  int numSheets = V_I4(v);

  fprintf(stderr, "Num sheets = %d\n", numSheets);fflush(stderr);

  for(int i = 0; i < numSheets ; i++) {
    v = &vars[4];
    V_VT(v) = VT_I4;
    V_I4(v) = i+1;
    fprintf(stderr, "Asking for Item %d\n", i);fflush(stderr);
    sheet = call(sheets, L"Item", v, 1);
    fprintf(stderr, "got it\n");fflush(stderr);
    loadSheet(sheet, gg, fileName);    
    sheet->Release();
  }
  sheets->Release();
  releaseVariants(vars, sizeof(vars)/sizeof(vars[0]), false);
  call(s, L"Close", NULL, 0);
#endif

  books->Release();
  iface->Release();
}

datad *
loadSheet(IDispatch *sheet, ggobid *gg, gchar *fileName)
{
  VARIANT vars[3], *v;
  IDispatch *cells;

  getProperty(sheet, L"UsedRange", v = &vars[0]);
    cells = V_DISPATCH(v);
  getProperty(cells, L"Value", v = &vars[1]);

  datad *d = createDataset(v, gg);
  getProperty(sheet, L"Name", v = &vars[2]);
  d->name = FromBstr(V_BSTR(v));

  releaseVariants(vars, sizeof(vars)/sizeof(vars[0]), true);
  return(d);
}


IDispatch *
getWorkbooks(IDispatch *iface)
{
  VARIANT v;
  HRESULT hr;
  IDispatch *books;

  getProperty(iface, L"Workbooks", &v);

  books = V_DISPATCH(&v);
  books->AddRef();
  VariantClear(&v);
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
  DISPID mid;
  HRESULT hr;
  DISPPARAMS params = {NULL, NULL, 0, 0};

  hr = iface->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &mid);
  if(FAILED(hr))
    Error("Can't get id of name");

 params.rgvarg = args;
 params.cArgs = numArgs;

 VariantInit(&v);

 hr = iface->Invoke(mid, IID_NULL, LOCALE_USER_DEFAULT, INVOKE_FUNC | INVOKE_PROPERTYGET, &params, &v, NULL, NULL);
 if(FAILED(hr)) {
    COMError(hr);
 }

 IDispatch *ans = NULL;
 if(V_VT(&v) == VT_DISPATCH) {
   ans = V_DISPATCH(&v);
   ans->AddRef();
 }
 VariantClear(&v);

 return(ans);
}



double
asReal(VARIANT *v)
{
  VariantChangeType(v, v, 0, VT_R8);
  return(V_R8(v));
}

datad *
createDataset(VARIANT *var, ggobid *gg)
{
  SAFEARRAY *arr;
  VARIANT value;
  gboolean hasColNames = true;

  if(V_ISBYREF(var))
    arr = *V_ARRAYREF(var);
  else
    arr = V_ARRAY(var);

  long lb, ub;
  long i, j, ctr, col;
  long indices[2];
  long dim[2][2];
  UINT numDim;
  datad *d = NULL;

  numDim = SafeArrayGetDim(arr); //  should be two - always!

  for(i = 0; i < 2; i++) {
    SafeArrayGetLBound(arr, i+1, &dim[i][0]);
    SafeArrayGetUBound(arr, i+1, &dim[i][1]);
  }

  indices[0] = dim[0][0];
  indices[1] = dim[1][0];
  SafeArrayGetElement(arr, indices, &value);
  hasColNames = (V_VT(&value) == VT_BSTR);
  VariantClear(&value);

  int nrow = dim[0][1] - dim[0][0] + (hasColNames ? 0 : 1);
  int ncol =  dim[1][1] - dim[1][0] + 1;

  d = datad_create(nrow, ncol, gg);

  for(j = dim[1][0], col = 0;  j <= dim[1][1]; j++, col++) {
    indices[1] =j;
    for(ctr = 0, i = dim[0][0] + 1; i <= dim[0][1]; i++, ctr++) {
      indices[0] =i;
      SafeArrayGetElement(arr, indices, &value);
      if(V_VT(&value) == VT_VOID) {
	/* This doesn't work. Instead we get a value of 0.00, not VT_VOID */
	fprintf(stderr, "Missing value %d, %d\n", ctr+1, col+1);fflush(stderr);
	d->raw.vals[ctr][col] = -1.0;
      } else
         d->raw.vals[ctr][col] = asReal(&value);

      if(ctr == 7 && col == 1) {
	fprintf(stderr, "(8,1): %d  %lf\n", V_VT(&value), d->raw.vals[ctr][col]); fflush(stderr);
      }

      VariantClear(&value);
    }

    gchar *varName = NULL;
    if(hasColNames) {
      indices[0] = dim[0][0];
      SafeArrayGetElement(arr, indices, &value);
      if(V_VT(&value) == VT_BSTR)
         varName = g_strdup(FromBstr(V_BSTR(&value)));
    }
    if(!varName)
      varName = g_strdup_printf("Var %d", col+1);
    GGOBI(setVariableName)(col, varName, true, d, gg);
  }

  datad_init(d, gg, 0);

  SafeArrayDestroyData(arr);

 return(d);
}


/* 
  Following taken from the Omegahat package RDCOMServer/src.
  Keep in sync. if changed, please.
*/



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

char *
FromBstr(BSTR str)
{
  char *ptr;
  DWORD len = wcslen(str);
  ptr = (char *) g_malloc((len+1)*sizeof(char));
  ptr[len] = '\0';
  DWORD ok = WideCharToMultiByte(CP_ACP, 0, str, len, ptr, len, NULL, NULL);
  if(ok == 0) {
    ptr = NULL;
  }
  return(ptr);
}


void
Error(char *str)
{
  fprintf(stderr, "%s\n", str);
  exit(1);
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
