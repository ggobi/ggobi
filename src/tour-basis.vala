/*

= Basis generation =

These classes generate new bases for the tour. The grand tour generates bases
at random, while the guided tour generates new bases which have a larger score
on some index.

The basis generation functions need to be able to look at the stage (e.g. to
calculate projection pursuit indices) and the variable state.

Ideas for other methods:

 * little tour
 * rocker
 * packed tour
 * fractal curve algorithm (E. Wegman and J. Solka. On some mathematics for
  visualizing high dimensional data. The Indian Journal of Statistics,
  64:429–452, 2002.)

*/
using GLib;
public class GGobi.TourBasis : Object {
  public uint d {construct; get;}
  public uint p = 0; 
  public Stage stage {construct; get;}
  public TourMatrix last_basis;
  
  public TourBasis(construct Stage stage, construct uint d) {}
  construct {
    p = stage.n_cols;
  }  

  // Add in frozen variables.  
  // This convenience function takes a generated matrix and combines it with
  // any frozen variables.  It is the responsibility of concrete subclasses
  // to deal with frozen variables correctly.
  public void add_frozen_vars(out TourMatrix mat, TourState[] states) {
    for(uint j = 0; j < mat.n_cols; j++) {

      // Calculate norm of frozen values
      double frozen_length = 0;
      for(uint i = 0; i < mat.n_rows; i++) {
        if (states[i] == TourState.FROZEN) 
          frozen_length += Math.pow(mat.get(i, j), 2);
      }
      frozen_length = Math.sqrt(frozen_length);
      
      for(uint i = 0; i < mat.n_rows; i++) {
        if (states[i] == TourState.IN) {
          // Scale thawed variables
          mat.set(i, j, mat.get(i, j) / (1 - frozen_length));
        } else if (states[i] == TourState.FROZEN) {
          // Set frozen variables
          mat.set(i, j, last_basis.get(i, j));
        }
      }
    }
  }
  


  public abstract TourMatrix generate(TourState[] states);
  
}

