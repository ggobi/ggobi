#source(system.file("examples", "gtkhtml.S", package="RGtk"))

library(RGtk)
library(Rggobi)
source("~/Projects/org/omegahat/R/Gtk/examples/gtkhtml.S")
source("~/Projects/org/omegahat/R/Gtk/examples/command.S")

# We need the routine RGGobi_getHelpMenu.
dyn.load("plugins/R/Rplugin.so")


getDisplayWidget <-
  #
  # This returns the reference to the widget
  # that is the displayd window. From this, we can
  # get the children and the individual plots.
  #
  # This is used in the ggobi.html file to embed the
  # ggobi plots
  #
function(dpy)
{
   w <- .Call("RGGobi_getDisplayWindow", dpy)
   
   class(w) <- gtkObjectGetClasses(w, check = FALSE)
   w
}

createRHelpPluginInstance <-
  #
  # This is the function to create the Help plugin.
  # It finds the Help menu and adds an entry to it to bring
  # up the ``Manual''. This is a 
  #
function(gg, inst, parameters, args)  
{
    # We add an item to the Help menu and then add a callback for it.
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
                           # Create the window only if the variable ggobiHelp
                           # doesn't exist.
                               if(!exists("ggobiHelp")) {
                                 h <- viewHtml(parameters[["start"]])
                                 assign("ggobiHelp", h, env = globalenv())
                                   # Arrange to remove the ggobiHelp variable
                                   # if the user closes the window
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

