/*
= Randomize column values =

Useful for visual imputation, but will need to be embedded into a larger
framework and UI for optimal usefulness.

*/

using GLib;

namespace GGobi {
  public enum RandomizationType {NONE, REPEAT, UNIQUE}
}

public class GGobi.StageRandomize : Stage {
  
  public RandomizationType[] randomization;
  public PipelineMatrix cache;
  
  public override void refresh_col_(uint j) {
    if (randomization[j] == RandomizationType.NONE) {
      return;
    }
    
    switch (randomization[j]) {
      case RandomizationType.REPEAT:
        for(uint i = 0; i < n_rows; i++)
          cache.set(i, j, parent.get_raw_value(rand_row(), j));
        break;
        
      case RandomizationType.UNIQUE:
        //FIXME: check that this algorithm is actually correct
        for(uint i = 0; i < n_rows; i++)
          cache.set(i, j, parent.get_raw_value(i, j));
        for(uint i = 0; i < n_rows; i++) {
          uint irand = rand_row();
          double tmp = cache.get(irand, j);
          cache.set(irand, j, cache.get(i, j));
          cache.set(i, j, tmp);
        }
        break;
    } 
  }
  
  public uint rand_row() {
    return (uint) Random.int_range(0, (int32) n_rows - 1);
  }
  
  public override double get_raw_value(uint i, uint j) {
    if (randomization[j] == RandomizationType.NONE) {
      return parent.get_raw_value(i, j);
    }
    return cache.get(i, j);
  }

  public override void set_raw_value(uint i, uint j, double value) {
    if (randomization[j] == RandomizationType.NONE) {
      parent.set_raw_value(i, j, value);
    }
    cache.set(i, j, value);  
  }  
  
  construct {
    cache = new PipelineMatrix();
    randomization.resize(0);
  }
  
  /* Process incoming change events */
  public override void process_outgoing(PipelineMessage msg) {
    base.process_outgoing(msg);
    randomization.resize((int) n_cols); 
    cache.process_message(msg, this, false);
  }
  
  public void set_randomization(uint j, RandomizationType value) {
    if (randomization[j] == value) return;
    randomization[j] = value;
    refresh_col(j);
    col_parameter_changed(j);
    flush_changes_here();
  }
  
  
}