/*

List of surfaces.
Factory method to create new surfaces
Transforms data to screen (and vice versa)

*/

class Canvas {
  
  List<surface> surfaces;
  Surface result_surface;
  SurfaceFactory factory;
  
  limits screen_x;
  limits screen_y;

  limits data_x;
  limits data_y;
  
  Surface create_surface() {
    s = factory.create();
    s.canvas = self;
    return(s);
  }
  
  int get_width() {
    return xlim.max - xlim.min;
  }
  int get_height() {
    return ylim.max - ylim.min;
  }
  
  int transform_x(double x) { 
    return (x - xlim.min) / xlim.max;
  }
  int transform_y(double y) {
    return (y - ylim.min) / ylim.max;
  }
  
  // Build single layer that has all layers composited
  // Responsibility of each layer to call as needed
  void recomposite() {
    
  }
  
  
}