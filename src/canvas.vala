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
  
  double transform_x(x) { 
    return (x - xlim.min) / xlim.max;
  }
  double transform_y(y) {
    return (y - ylim.min) / ylim.max;
  }
  
  // Build single layer that has all layers composited
  // Responsibility of each layer to call as needed
  void recomposite() {
    
  }
  
  
}