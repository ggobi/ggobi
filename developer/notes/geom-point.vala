class GeomPoint : Geom {
  
  StageFlatten stage;
  
  void render(Surface surface) {
    surface.draw_points(stage.get_xs(), stage.get_ys());
  }
  
  void nearest_observations(double x, double y) {
    
  }

  void observations_in_rectangle(double x1, double x2, double y1, double y2) {
    
  }
  
  
  // For crosshair for edge brushing
  void observations_on_segment(double x1, double x2, double y1, double y2) {
    
  }
  
  
  
}