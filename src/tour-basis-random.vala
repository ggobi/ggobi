/* 
= Random basis =

Generate new random bases for the grand tour.

*/

public class GGobi.TourBasisRandom : Object {
  public uint d = 2 {get; construct;}
  public uint p = 0; 
  public TourMatrix last_basis;
  public Stage stage {get; construct;}
  
  public TourBasisRandom(construct Stage stage, construct uint d) {}
  construct {
    p = stage.n_cols;
  }

  override TourMatrix generate(TourState[] states) {
    TourMatrix out = raw_matrix(states);

    // FIXME: Orthogonalise, preserving values of frozen variables
    // I think we can assume the frozen variables are orthogonal, and then
    // all other variables should be orthogonalised based on each frozen var
    // in turn, and then on each other.
    // 
    // Frozen variables should probably be dealt with a higher level, since
    // the methods are common to all basis generation functions.  The 
    // generator should just treat them as out, and then we'll add them in
    // later.
    out.orthonormalise_self();
    last_basis = out;
    
    return out;
  }
  
  TourMatrix raw_matrix(TourState[] states) {
    TourMatrix out = new TourMatrix(stage.n_cols, d);
    
    for(int i = 0; i < p; i++) {
      for(int j = 0; j < d; j++) {
        double value;
        switch (states[i]) {
          case TourState.IN: value = random_normal(); break;
          case TourState.OUT: value = 0; break;
          case TourState.FROZEN: value = last_basis.get(i, j); break;
        }
        out.set(i, j, value);
      }
    }
    
    return out;
  }
}