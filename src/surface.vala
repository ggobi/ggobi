// For now is a pixmap
// Eventually will become something different depending on the graphics
// toolkit that we use eventually
class Surface {
  
  // Canvas that this surface belongs to
  Canvas canvas;
  
  // 
  GdkPixmap buffer;
  
  // Each gdk method delegated to original method, with
  // positions transformed
  
  void draw() {
    ...;
    canvas.recomposite();
  }
  
  // 
  
  void draw_point() {}
  
}