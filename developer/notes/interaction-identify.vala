/* 

Needs efficient access to nearest observations(s) to a point.

Complication: sticky points and nearest points need to be made available to all points on the pipeline.

Drawing layers:
  * layer for diamonds and text labels

*/


class InteractionIdentify : Interaction {
  Layer p;
  Canvas c;
  Surface s;
  construct Stage s;  // initialised when identify mode created
  
  VarSelection varsel; // set by identify gui
  
  public InteractionIdentify(Plot p) {
    s.create_attribute("labelled");
    s.create_attribute("under_mouse");

    s = c.create_surface()
  }
  
  void activate() {
    
  }
  
  void deactivate() {
    s.set_data_attribute("nearest_point", -1)
    redraw();
  }
  

  void mouse_move(double x, double y) {
    // shouldn't actually be nearest points
    int nearest_point = c.get_nearest_point(x, y);
    s.set_data_attribute("nearest_point", nearest_point)
    
    redraw();
  }
  
  void mouse_down(double x, double y) {
    sticky_points.add(nearest_point);
    redraw();
  }
   
  void redraw() {
    l.clear();
    
    if (nearest_point >= 0) {
      l.draw_diamond(c.get_location(nearest_point));      
    }

    foreach(point in sticky_points) {
      l.draw_label(c.get_location(point), make_label(point));
    } 
  }
  
  string make_label(uint i) {
    
  }
  
}