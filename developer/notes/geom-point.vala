/* 


*/


class GeomPoint : Geom {
  
  StageFlatten stage;
  
  void render(Surface surface) {
    surface.draw_points(stage.get_xs(), stage.get_ys());
  }
  
 
  
  
}