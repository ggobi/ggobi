#include <windows.h>
#include <oaidl.h>
#include <oleauto.h>

#include <stdio.h>

extern "C" { 
#include "ggobi.h"
  //#include "externs.h"
#include "GGobiAPI.h"
#include "plugin.h"

#include "externs.h"

displayd*  datad_init (datad *, ggobid *, gboolean);

extern const gchar **getDefaultRowNamesPtr();

InputDescription *ExcelDataDescription(const char * const fileName, const char * const modeName, ggobid *gg, GGobiPluginInfo *info);
gboolean onLoad(gboolean initializing, GGobiPluginInfo *plugin);
}


gboolean isVTNumber(VARIANT *v);

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
    return(NULL);
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
#ifdef DEBUG_EXCEL_PLUGIN
  fprintf(stderr, "Closed() the sheet\n"); fflush(stderr);
#endif

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

#ifdef DEBUG_EXCEL_PLUGIN
  fprintf(stderr, "Num sheets = %d\n", numSheets);fflush(stderr);
#endif

  for(int i = 0; i < numSheets ; i++) {
    v = &vars[4];
    V_VT(v) = VT_I4;
    V_I4(v) = i+1;
#ifdef DEBUG_EXCEL_PLUGIN
    fprintf(stderr, "Asking for Item %d\n", i);fflush(stderr);
#endif
    sheet = call(sheets, L"Item", v, 1);
#ifdef DEBUG_EXCEL_PLUGIN
    fprintf(stderr, "got it %p\n",sheet);fflush(stderr);
#endif
    if(!sheet) {
#ifdef DEBUG_EXCEL_PLUGIN
      fprintf(stderr, "skipping %d\n", i);fflush(stderr);
#endif
      continue;
    }
    loadSheet(sheet, gg, fileName);    
    sheet->Release();
  }
  sheets->Release();
  releaseVariants(vars, sizeof(vars)/sizeof(vars[0]), false);
  call(s, L"Close", NULL, 0);
#endif

#ifdef DEBUG_EXCEL_PLUGIN
  fprintf(stderr, "Releasing books\n");fflush(stderr);
#endif
  books->Release();
  iface->Release();

#ifdef DEBUG_EXCEL_PLUGIN
  fprintf(stderr, "Closing down Excel\n");fflush(stderr);
#endif
}

datad *
loadSheet(IDispatch *sheet, ggobid *gg, gchar *fileName)
{
  VARIANT vars[3], *v;
  IDispatch *cells;

  getProperty(sheet, L"UsedRange", v = &vars[0]);
    cells = V_DISPATCH(v);
  getProperty(cells, L"Value", v = &vars[1]);

  if(!v || V_VT(v) == VT_NULL) {
    fprintf(stderr, "Null value passed back for used range\n");fflush(stderr);
    releaseVariants(vars, sizeof(vars)/sizeof(vars[0]), true);
    return(NULL);
  }
  datad *d = createDataset(v, gg);
  if(d) {
#ifdef DEBUG_EXCEL_PLUGIN
    fprintf(stderr, "Finished getting dataset\n");fflush(stderr);
#endif
    getProperty(sheet, L"Name", v = &vars[2]);
    d->name = FromBstr(V_BSTR(v));
  }

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

/*XXX
 Taken from read_xml.c. Merge back!
*/
void
freeLevelHashEntry(gpointer key, gpointer value, gpointer data)
{
  g_free(value);
  if(data)
    g_free(key);
/*  return(true); */
}

typedef struct {
  int ctr;
  vartabled *var;
} HashElSet;

void
setLevel(gpointer key, gint *value, HashElSet *tmp)
{
  tmp->var->level_names[tmp->ctr] = g_strdup((gchar *) key);
  tmp->var->level_values[tmp->ctr] = value[0];
  tmp->var->level_counts[tmp->ctr] = value[1];
  tmp->ctr++;
}

/* Take the elements in the hashtable and put them into 
   the level fields of the vartabled object.
*/
void
setLevels(vartabled *var, GHashTable *levels, int numLevels)
{
  int ctr = 0, n;
  HashElSet tmp;
  
  tmp.ctr = 0;
  tmp.var = var;

  n = var->nlevels = numLevels;
  var->level_values = (gint *) g_malloc(sizeof(gint) * n);
  var->level_counts = (gint *) g_malloc(sizeof(gint) * n);
  var->level_names = (gchar **) g_malloc(sizeof(gchar *) * n);
  
  g_hash_table_foreach(levels, (GHFunc) setLevel, &tmp);
}

datad *
createDataset(VARIANT *var, ggobid *gg)
{
  SAFEARRAY *arr;
  VARIANT value;
  gboolean hasColNames = true;
  vartabled *variable;
  GHashTable *levels = NULL;
  int numLevels;

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

#ifdef DEBUG_EXCEL_PLUGIN
  fprintf(stderr, "Num dimensions %d\n", (int) numDim);fflush(stderr);
#endif

  for(i = 0; i < 2; i++) {
    SafeArrayGetLBound(arr, i+1, &dim[i][0]);
    SafeArrayGetUBound(arr, i+1, &dim[i][1]);
  }

#ifdef DEBUG_EXCEL_PLUGIN
  fprintf(stderr, "Dimensions: %d %d %d %d\n", 
	  (int) dim[0][0], (int) dim[0][1],
	  (int) dim[1][0], (int) dim[1][1]);fflush(stderr);
#endif

    /* Degenerate/empty range. */
  if(dim[0][0] == dim[0][1]) {
    return(NULL);
  }

  indices[0] = dim[0][0];
  indices[1] = dim[1][0];
  SafeArrayGetElement(arr, indices, &value);
  hasColNames = (V_VT(&value) == VT_BSTR);
  VariantClear(&value);

  int nrow = dim[0][1] - dim[0][0] + (hasColNames ? 0 : 1);
  int ncol =  dim[1][1] - dim[1][0] + 1;

  d = datad_create(nrow, ncol, gg);

  /* Loop over columns. */
  for(j = dim[1][0], col = 0;  j <= dim[1][1]; j++, col++) {
    variable = vartable_element_get (col, d);
    
    indices[1] =j;
    for(ctr = 0, i = dim[0][0] + 1; i <= dim[0][1]; i++, ctr++) {
      indices[0] =i;
      SafeArrayGetElement(arr, indices, &value);
      if(V_VT(&value) == VT_VOID) {
	/* This doesn't work. Instead we get a value of 0.00, not VT_VOID */
	fprintf(stderr, "Missing value %d, %d\n", ctr+1, col+1);fflush(stderr);
	d->raw.vals[ctr][col] = -1.0;
      } else if(isVTNumber(&value)) {
        d->raw.vals[ctr][col] = asReal(&value);
      } else if(V_VT(&value) == VT_BSTR) {
	gpointer ptr;
	int *val;
	gchar *str;

        str = g_strdup(FromBstr(V_BSTR(&value)));

	/* If first row of this column, set this to a categorical variable
           and create a table in which to store the levels. We will use this
           for the cells in this column and then put the unique elements into
           the variable at the end. */
	if(ctr == 0) {
	  numLevels = 0;
	  ptr = NULL;
	  variable->vartype = categorical;
	  levels = g_hash_table_new(g_str_hash, g_str_equal);
	} else {
	  /* Else, see if this level is already entered in the set of levels. */
	  ptr = g_hash_table_lookup(levels, str);
	}

	/* If the value for the level is not in the set, add it.*/
	if(!ptr) {
	    val = (int*) malloc(sizeof(gint) * 2);
	    numLevels++;
	    val[0] = numLevels;
	    val[1] = 0;
	    g_hash_table_insert(levels, str, val);
	} else
	  val = (int *)ptr;

	d->raw.vals[ctr][col] = val[0];
	val[1]++;
      } else {
	fprintf(stderr, "Skipping entry (%d, %d)\n", ctr+1, col+1);fflush(stderr);
      }

      VariantClear(&value);
    }

    if(levels) {
      setLevels(variable, levels, numLevels);
      g_hash_table_foreach(levels, freeLevelHashEntry, NULL);
      g_hash_table_destroy(levels);
      levels = NULL;
    }

    gchar *varName = NULL;
    if(hasColNames) {
      indices[0] = dim[0][0];
      indices[1] = col; // dim[0][0];
      SafeArrayGetElement(arr, indices, &value);
      if(V_VT(&value) == VT_BSTR)
         varName = g_strdup(FromBstr(V_BSTR(&value)));

      VariantClear(&value);
    }

    if(!varName)
      varName = g_strdup_printf("Var %d", col+1);

    GGOBI(setVariableName)(col, varName, false, d, gg);
  }

  datad_init(d, gg, 0);

  SafeArrayDestroyData(arr);

 return(d);
}


gboolean
isVTNumber(VARIANT *v)
{
  VARTYPE t = V_VT(v);

  if(t == VT_I2 || t == VT_I4 || t == VT_R4 || t == VT_R8
      || t == VT_UI1 || t == VT_UI2 || t == VT_UI4 || t == VT_I8
        || t == VT_UI8 || t == VT_INT || t == VT_UINT || t == VT_DATE)
    return(true);

  return(false);
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
