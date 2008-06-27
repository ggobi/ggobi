/* 

Interaction modes ------------------------------------------------------------

Interaction modes convert user events (mouse and keyboard events on plot gui) into changes to the plot (pipeline, canvas or surfaces).

For large datasets, should be able to only send updates down pipeline on mouse up.  (General property of all interaction modes - control over when messages are streamed.)


*/


abstract class Interaction : Glib.Object {
  
  // Interaction modes must have access to plot, because this
  // mediates the result of all actions.
  Plot p;
  
  
  // Activations and deactivation.  Occurs for two reasons: when active
  // plot or active interaction plot changes
  abstract void activate();
  abstract void deactivate();
  
  
  // The appearance of the interaction mode should change depending
  // on whether or not the mode is active.  For example, the brush
  // rectangle should be drawn when brush mode is active, but sticky points
  // should be drawn even when identify mode is inactive.
  abstract void redraw();
  
}