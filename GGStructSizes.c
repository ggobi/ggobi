typedef struct {
  unsigned int size;
  const char * const name;
} GGobi_StructSize;

#define GG_StructEntry(type) {sizeof(type), #type}

static const GGobi_StructSize  ggobiStructs[] = {
	GG_StructEntry(ggobid),
	GG_StructEntry(datad),
	GG_StructEntry(displayd),
	GG_StructEntry(splotd), 
	GG_StructEntry(vartabled) 
};

const GGobi_StructSize *
#ifdef GGOBI_MAIN
GGOBI(getStructs)(int *n)
#else
GGOBI(getGGobiStructs)(int *n)
#endif
{
   *n = sizeof(ggobiStructs)/sizeof(ggobiStructs[0]);
   return(ggobiStructs);
}
