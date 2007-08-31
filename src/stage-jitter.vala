/* 
= Jittering =

When displaying variables with few unique values (eg. discrete) on a scatterplot, it is useful to add a small amount of random jitter. 

In GGobi2, jittering occured after the world transformation, so it could assume that the range of each variable was [0, 1].  This is not the case in GGobi3, so the range of the variable is used explicity. 

Jittered value value = original * (1 - amount) + random [-range, range] * amount

*/
using GLib;

public class GGobi.StageJitter : Stage {
  /* Degree of jittering (between 0 and 1) for each column */
  public double[] amount;

  /* Is the distribution uniform or random? */
  public bool uniformDist = true;
  
  /* Cache of random values, with range equal to the range of each column */
  public double[] cache;
  
  /* Recompute cache of random variables */
  public void reset() {
    for (uint j = 0; j < n_cols; j++) reset_col(j);
  }
  public void reset_col(uint j) {
    float range = get_variable(j).get_range();
    for (uint i = 0; i < n_rows; i++) 
      cache[i][j] = rand() * range;

    col_data_changed(j);
    flush_changes_here();
  }
  
  /* Generate random number from specified distribution */
  double rand() {
    if (uniformDist) {
      return g_random_double_range(-1, 1);
    } else {
      return random_normal();
    }
  }
  
  /* Jitter values are the original value + jittering */
  override double get_raw_value(uint i, uint j) {
    double original = parent.get_raw_value(i, j);
    if (amount[j] == 0) return original;
    
    return original * (1 - amount[j]) + cache[i][j] * amount[j];
  }

  /* When setting the value of a jittered observation, subtract off the
  jittering first */
  override void set_raw_value(uint i, uint j, double value) {
    if (amount[j] == 0) {
      parent.set_raw_value(i, j, value);
      return;
    }

    double original;
    original = (value - cache[i][j] * amount[j]) / (1 - amount[j]);
    parent.set_raw_value(i, j, original);
  }  
  
  /* Process incoming change events */
  override void process_incoming(PipelineMessage msg) {
    SList removed_rows = msg.get_removed_rows();
    SList removed_cols = msg.get_removed_cols();
    uint n_added_cols = msg.get_n_added_cols();
    uint n_added_rows = msg.get_n_added_rows();
    uint n_refresh = n_added_cols;
    
    // PARENT_HANDLER(self, msg); // ?
    
    
    
    // Add amounts for new columns (set to zero)
    amount.resize(amount.size + n_added_cols);

    // Add cached jittered values for new column (set to rand())
    cache.resize(cache.size + n_added_cols);
    for (uint i = 0; i < n_added_cols; i++) {
      reset_col(cache.size + i);
    }
  }
}