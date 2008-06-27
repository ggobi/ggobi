/*

The brush is pretty complex because it's so configurable

Needs efficient access to observations within a rectangle.

Drawing layers:
  * points in current brush colour
  * brush rectangle

Complication: brush also needs to access other datasets when categorical brushing is enabled.

Colour, size and glyph defaults are global.  Categorical linking variable is local to the plot (but maybe should be local to the dataset).

*/

class InteractionBrush : Interaction {
  
  bool transient;
  int width;
  int height;
  
  Color color;
  Size size;
  Glyph glyph;
  
  BrushType edges;
  BrushType points;
  
  
}