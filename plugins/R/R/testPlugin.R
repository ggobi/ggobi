cat("Ok, in testPlugin.R\n")
ggobiTestPlugin <-
function(gg, inst, namedArgs, args)
{
 print(gg)
 print(inst)
 print(namedArgs)
 print(args)
 onClose <- function() {
   cat("In onClose\n")
 }

 onUpdateDisplay <- function() {
   cat("In onUpdateDisplay\n")
   print(gg)
 }


 cat("In ggobiTestPlugin()\n")
 v <- list(
           onClose = onClose,
           onUpdateDisplay = onUpdateDisplay)

 v
}  

cat("Defined ggobiTestPlugin\n")


