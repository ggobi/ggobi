/* 

The geoms are responsible for drawing observations (in some way) on the rendering surface, and for looking up coordinates in the data space and matching them up to observations in the data.

Most geoms are simple, but for performance reasons may use more sophisticated data structures to avoid a linear scan through the stage data to identify points that have been interacted with on the plot).  See e.g. quad trees: http://en.wikipedia.org/wiki/Quadtree

*/


abstract class GeomPoint : Geom {
  
  StageFlatten stage;
  
  abstract void render(Surface surface);
  
  
  // The following methods provide the interface for locating observations
  // based on their position in the data space.  If the geom provides 
  abstract void nearest_observations(double x, double y);

  abstract void observations_in_region(double x1, double x2, double y1, double y2);
    
  abstract void observations_on_segment(double x1, double x2, double y1, double y2);
  
  
  
}