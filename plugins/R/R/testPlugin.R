cat("Ok, in testPlugin.R\n")
ggobiTestPlugin <-
function(gg, inst)
{
 print(gg)
 print(inst)
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


