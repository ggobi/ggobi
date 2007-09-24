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
  64:429–452, 2002.
)

*/

public class GGobi.TourBasis : Object {
  
  public abstract TourMatrix generate(Stage stage, TourState[] states);
 
  public add_frozen_vars(TourMatrix m, TourState[] states) {
    
  }
  
}

