/*

List of surfaces.
Factory method to create new surfaces
Transforms data to screen (and vice versa)

*/

class Canvas {
  
  List<surface> surfaces;
  Surface buffer;
  
  // Store screen and data limits
  int width {get;};
  int height {get;};

  Limits data_x;
  Limits data_y;
  
  Surface create_surface() {
    return new Surface(this);
  }
  
  int transform_x(double x) { 
    return (int) ((x - xlim.min) / xlim.max * width);
  }
  int transform_y(double y) {
    return (int) ((y - ylim.min) / ylim.max * height);
  }
  
  void resize(int width, int height) {
    this.width = width;
    this.height = height;
    
    foreach(surface in surfaces) {
      surface.resize();
    }
  }
  
  // Build single layer that has all layers composited
  // Responsibility of each layer to call as needed
  void recomposite() {
    buffer.clear();
    foreach(surface in surfaces) {
      surface.render(buffer);
    }
  }
  
  void render(GdkDrawable target) {
    buffer.render_to_drawable(target)
  }
  
  
}