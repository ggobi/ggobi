#source(system.file("examples", "gtkhtml.S", package="RGtk"))

library(RGtk)
source("~/Projects/org/omegahat/R/Gtk/examples/gtkhtml.S")
source("~/Projects/org/omegahat/R/Gtk/examples/command.S")

# We need the routine RGGobi_getHelpMenu.
dyn.load("plugins/R/Rplugin.so")

createRHelpPluginInstance <-
  #
  # This is the function for 
  #
  #
function(gg, inst, parameters, args)  
{
    # We add an item to the Help menu and then add a callback for it.
print(args)
    # First we get the Help menu
  help <- .Call("RGGobi_getHelpMenu", gg)
  class(help) <- gtkObjectGetClasses(help)

    # Create a separator and then add the `Manual' menu item
  help$Append( gtkMenuItemNew())
  item <- gtkMenuItem("Manual")
    # Next we add a callback for when the user clicks on this menu item
    # and we arrange to show the help pages. The first page to show
    # is specified in the paramters[["start"]] variable
  gtkObjectAddCallback(item, "activate", function(w) {
                               if(!exists("ggobiHelp")) {
                                 h <- viewHtml(parameters[["start"]])
                                 assign("ggobiHelp", h, env = globalenv())
                                 h$win$AddCallback("delete-event",
                                                   function(w, ev) {
                                                     remove("ggobiHelp", envir=globalenv())
                                                     FALSE
                                                   })
                               }
                              })

  help$Append(item)
 
 list(destroy = function(){ cat("Closing help\n") },
      update = function() { cat("Updating display\n")})
}  

