/* 
= Random basis =

Generate new random bases for the grand tour.

*/

public class GGobi.TourBasisRandom : TourBasis {

  override TourMatrix generate(TourState[] states) {
    TourMatrix mat = raw_matrix(states);
    add_frozen_vars(out mat, states);
    
    last_basis = mat;
    return mat;
  }
  
  // Generate a random projection, by orthonormalising a projection drawn
  // from a multivariate normal.
  TourMatrix raw_matrix(TourState[] states) {
    TourMatrix mat = new TourMatrix(stage.n_cols, d);
    
    for(int i = 0; i < p; i++) {
      for(int j = 0; j < d; j++) {
        double value = 0;
        if (states[i] == TourState.IN) {
          value = Utils.random_normal();
        }
        mat.set(i, j, value);
      }
    }
    mat.orthogonalize();
    mat.normalize();
    return mat;
  }
}