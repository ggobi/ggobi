library(RGtk)

fileSelectPlugin <-
function(gg, plugin, dir = getwd())
{
 dlg <- gtkFileSelection("GGobi file selection using R")
 dlg$Show() 

 onClose <- function() {
   cat("[fileSelectPlugin] onClose\n")   
   dlg <- NULL
 }
 
 cat("Creating fileSelectPlugin\n")

 return(list(onClose))
}  
