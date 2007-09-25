/* 
= Jittering =

When displaying variables with few unique values (eg. discrete) on a
scatterplot, it is useful to add a small amount of random jitter.

In GGobi2, jittering occured after the world transformation, so it could
assume that the range of each variable was [0, 1]. This is not the case in
GGobi3, so the range of the variable is used explicity.

Jittered value value = original * (1 - amount) + random [-min, max] *
amount

*/
using GLib;

public class GGobi.StageJitter : Stage {
  /* Degree of jittering (between 0 and 1) for each column */
  public double[] amount;

  /* Is the distribution uniform or random? */
  public bool uniformDist = true;
  
  /* Cache of random values, with range equal to the range of each column */
  public PipelineMatrix cache;
  
  public override void refresh_col_(uint j) {
    Variable v = get_variable(j);
    
    double range = v.get_range();
    double mid = (v.get_min() + v.get_max()) / 2;
    
    for (uint i = 0; i < n_rows; i++) 
      cache.set(i, j, mid + rand() * range);
  }
  
   
  /* Generate random number from specified distribution */
  double rand() {
    if (uniformDist) {
      return Random.double_range(-0.5, 0.5);
    } else {
      return Utils.random_normal() / 1.5;
    }
  }
  
  /* Jitter values are the original value + jittering */
  override double get_raw_value(uint i, uint j) {
    double original = parent.get_raw_value(i, j);
    if (amount[j] == 0) return original;
    
    return original * (1 - amount[j]) + cache.get(i, j) * amount[j];
  }

  /* When setting the value of a jittered observation, subtract off the
  jittering first */
  override void set_raw_value(uint i, uint j, double value) {
    double original;
    if (amount[j] == 0) {
      original = value;
    } else {
      original = (value - cache.get(i, j) * amount[j]) / (1 - amount[j]);
    }
    parent.set_raw_value(i, j, original);
  }  
  
  construct {
    cache = new PipelineMatrix();
    amount.resize(0);
  }
  
  /* Process incoming change events */
  override void process_outgoing(PipelineMessage msg) {
    base.process_outgoing(msg);
    cache.process_message(msg, this);
    amount.resize((int) n_cols); 
  }
  
  public void update_amounts(SList<uint> cols, double value) {
    foreach(uint j in cols) {
      amount[j] = value;
      col_data_changed(j);
      col_parameter_changed(j);
    }
    flush_changes_here();
  }
}