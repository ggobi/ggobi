#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

EXTERN_C void boot_DynaLoader (CV* cv);
EXTERN_C void boot_plugin(CV* cv);


EXTERN_C void
xs_init()
{
             char *file = __FILE__;
             /* DynaLoader is a special case */
             newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
             newXS("GGobi::bootstrap", boot_plugin, file);
}  
