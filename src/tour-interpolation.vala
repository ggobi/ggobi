/* 

= Tour interpolation =

Interpolates between two frames, using geodesic path, as outlined in
http://www-stat.wharton.upenn.edu/~buja/PAPERS/paper-dyn-proj-algs.pdf and
http://www-stat.wharton.upenn.edu/~buja/PAPERS/paper-dyn-proj-math.pdf

Need to decide whether this should be a singleton class, or contain state
about the current place in the interpolation.  

*/

class GGobi.TourInterpolation : Object {
  public TourMatrix from;
  public TourMatrix to;
  
  public double stepsize;
  
  // Distance between to and from projection matrices
  public double dist();
  
  public TourMatrix get_next();
  public bool is_finished();
  public void reset();
  
  public TourMatrix get_interp(double percent);

  public void set_target(TourMatrix target) {
    from = to;
    to = target;
    reset();
  }
  
  
}