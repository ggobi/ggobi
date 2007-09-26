/*
= Projection pursuit (guided tour) =

Guided tour uses simulated annealing to select new bases that are more than
the current projection.

*/

namespace GGobi {
  public delegate double PursuitIndex(Matrix mat, Stage stage);
}

public class GGobi.TourBasisPursuit : TourBasis {
  public PursuitIndex index_function;
  public double current_index;
  
  // Current temperature
  public double temp = 1;
  // Multiplicative cooling factor, applied after each success
  public double cooling = 0.99;
  
  // The number of new projections that we have tried
  private uint tries = 0;
  // The number of new projections better than the previous projection
  private uint successes = 0;
  
  private double     new_index;
  private TourMatrix new_data;
  private TourMatrix new_basis;
  private TourMatrix current_basis;
  
  public double compute_index(TourMatrix mat) {
    return index_function(mat.matrix, stage);
  }
  
  public void reset() {
    temp = 1;
    tries = 0;
    successes = 0;
  }

  public TourMatrix generate_possibility(double temp, TourState[] states) {
    TourMatrix mat = current_basis.copy();
    
    // We don't need a convex combination here because mat is orthonormal
    for(uint i = 0; i < p; i++) {
      if (states[i] == TourState.IN) {
        for(uint j = 0; j < d; j++) {
          mat.set(i, j, mat.get(i, j) + temp * Utils.random_normal());
        }
      }
    }
    add_frozen_vars(out mat, states);
    mat.orthogonalise();
    mat.normalise();
    
    return mat;
  }

  // Returns true if new target is better than current.  
  // New basis, data and index stored as properties of this object.
  public bool step(TourState[] states) {
    tries++;

    new_basis = generate_possibility(temp, states);
    new_data  = new_basis.project(stage);
    new_index = compute_index(new_data);

    // Never step when the new index is worse.  This is different from classic
    // simulated annealing as we can rely on the user to get out of local
    // maxima
    return(new_index > current_index);
  }
  
  override TourMatrix generate(TourState[] states) {
    // Do at most 100 iterations
    for(uint i = 0; i < 100; i++) {
      if (step(states)) break;
      return(current_basis);
    }
    successes++;
    temp *= cooling;
    
    
    current_basis = new_basis;
    current_index = new_index;
    return new_basis;
  }

}