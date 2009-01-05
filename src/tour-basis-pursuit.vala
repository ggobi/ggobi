/*
= projected_data pursuit (guided tour) =

Guided tour uses simulated annealing to select new bases that are more than
the current projected_data.

*/

using GLib;

namespace GGobi {
  public delegate double PursuitIndex(Matrix mat, Vector groups);
}

public class GGobi.TourBasisPursuit : TourBasis {
  public PursuitIndex index_function;

  // Multiplicative cooling factor, applied after each success
  public double cooling = 0.99;

  // Maximum number of iterations
  public uint max_iter = 100;

  // Current temperature
  private double temp = 1;
  
  // The number of new projected_datas that we have tried
  private uint tries = 0;

  // The number of new projected_datas we have stepped to
  private uint steps = 0;

  // Should the simulated annealing take steps that worsen the index function
  // (true) or should we rely on the user to step out of local optima?
  // (false, default)
  public bool worsen = false;
  
  private double     current_index;
  private double     new_index;

  private TourMatrix current_basis;
  private TourMatrix new_basis;

  private TourMatrix projected_data;
  
  public double compute_index(TourMatrix mat) {
    Vector groups = new Vector();
    return index_function(mat.matrix, groups);
  }
  
  public void reset() {
    temp = 1;
    tries = 0;
    steps = 0;
  }

  public TourMatrix generate_possibility(double temp, TourState[] states) {
    TourMatrix mat = current_basis.copy();
    
    // Needs to be replaced with better sampling algorithm
    for(uint i = 0; i < p; i++) {
      if (states[i] == TourState.IN) {
        for(uint j = 0; j < d; j++) {
          mat.set(i, j, mat.get(i, j) + temp * Utils.random_normal());
        }
      }
    }
    mat.orthogonalize();
    mat.normalize();
    add_frozen_vars(mat, states);
    
    return mat;
  }

  // Returns true if we take a step
  // New basis, data and index stored as properties of this object.
  public bool step(TourState[] states) {
    tries++;

    new_basis = generate_possibility(temp, states);
    projected_data  = new_basis.project(stage);
    new_index = compute_index(projected_data);

    if (new_index > current_index) return true;

    if (worsen) {
      double value = (current_index - new_index) / temp * Math.log(steps + 1);
      return Random.next_double() > value;
    } else {
      return false;  
    }
    
  }
  
  public override TourMatrix generate(TourState[] states) {
    // Limit number of iterations
    for(uint i = 0; i < max_iter; i++) {
      if (step(states)) break;
      return(current_basis);
    }
    steps++;
    temp *= cooling;
    
    current_basis = new_basis;
    current_index = new_index;
    return new_basis;
  }

}