// For now is a pixmap
// Eventually will become something different depending on the graphics
// toolkit that we use eventually
class Surface {
  
  // Canvas that this surface belongs to
  Canvas canvas;
  
  // Off screen buffer where all drawing is rendered
  GdkPixmap buffer;

  public Surface(Canvas canvas) {
    this.canvas = canvas;
    resize()
  }
  
  void resize() {
    buffer = new GdkPixmap(NULL, canvas.width, canvas.height, 8);
  }
  
  // Render the contents of this surface onto another
  void render(Surface surface) {
    
  }
  
  void render_to_drawable(GdkDrawable drawable) {
    
  }
  
  // Each gdk method delegated to original method, with
  // positions transformed
  void draw_point(GdkGC gc, double x, double y) {
    buffer.draw_point(gc, canvas.transform_x(x), canvas.transform_y(y));
  }
  
  // Methods currently used by GGobi:
  //    1 gdk_draw_points
  //    1 gdk_gc_set_background
  //    2 gdk_draw_drawable
  //    2 gdk_draw_point
  //    2 gdk_draw_string
  //    2 gdk_gc_destroy
  //    3 gdk_draw_lines
  //    4 gdk_draw_polygon
  //    4 gdk_gc_set_dashes
  //    6 gdk_draw_segments
  //   15 gdk_draw_pixmap
  //   22 gdk_draw_arc
  //   30 gdk_draw_layout
  //   35 gdk_gc_set_line_attributes  
  //   42 gdk_draw_rectangle
  //   54 gdk_draw_line
  //   88 gdk_gc_set_foreground

  void recomposite() {
    canvas.recomposite();
  }
  
  
}