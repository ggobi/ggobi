#define OPEN_LIST(fp)  fprintf(fp, "list(")
#define OPEN_C(fp)     fprintf(fp, "c(")

#define OPEN_NAMED_LIST(fp, name)  fprintf(fp, "%s = list(", name)
#define OPEN_NAMED_C(fp, name)     fprintf(fp, "%s = c(", name)

#define CLOSE_LIST(fp) fprintf(fp, ")")
#define CLOSE_C(fp)    fprintf(fp, ")")

#define ADD_COMMA(fp)  fprintf(fp, ",")
#define ADD_CR(fp)        fprintf(fp, "\n");
