/* 
= Jittering =

When displaying variables with few unique values (eg. discrete) on a
scatterplot, it is useful to add a small amount of random jitter.

In GGobi2, jittering occured after the world transformation, so it could
assume that the range of each variable was [0, 1]. This is not the case in
GGobi3, so the range of the variable is used explicity.

Jittered value value = original * (1 - amount) + random [-range, range] *
amount

*/
using GLib;

public class GGobi.StageJitter : Stage {
  /* Degree of jittering (between 0 and 1) for each column */
  public double[] amount;

  /* Is the distribution uniform or random? */
  public bool uniformDist = true;
  
  /* Cache of random values, with range equal to the range of each column */
  public Matrix cache;
  
  /* Recompute cache of random variables */
  public void refresh() {
    for (uint j = 0; j < n_cols; j++) refresh_col(j);
  }
  public void refresh_col(uint j) {
    float range = get_variable(j).get_range();
    for (uint i = 0; i < n_rows; i++) 
      ((double[]) cache.vals[i])[j] = rand() * range;
    
    col_data_changed(j);
    flush_changes_here();
  }
  
  /* Generate random number from specified distribution */
  double rand() {
    if (uniformDist) {
      return Random.double_range(-1, 1);
    } else {
      return Utils.random_normal();
    }
  }
  
  /* Jitter values are the original value + jittering */
  override double get_raw_value(uint i, uint j) {
    double original = parent.get_raw_value(i, j);
    if (amount[j] == 0) return original;
    
    return original * (1 - amount[j]) + ((double[]) cache.vals[i])[j] * amount[j];
  }

  /* When setting the value of a jittered observation, subtract off the
  jittering first */
  override void set_raw_value(uint i, uint j, double value) {
    if (amount[j] == 0) {
      parent.set_raw_value(i, j, value);
      return;
    }

    double original;
    original = (value - ((double[]) cache.vals[i])[j] * amount[j]) / (1 - amount[j]);
    parent.set_raw_value(i, j, original);
  }  
  
  /* Process incoming change events */
  override void process_incoming(PipelineMessage msg) {
    // Deal with jittering amounts

    // Set up and reinitialise cache matrix    
    uint n_added_cols = msg.get_n_added_cols();
    uint current_cols = cache.n_cols;
    cache.add_cols((int) n_added_cols);

    cache.add_rows((int) msg.get_n_added_rows());
    cache.remove_rows(msg.get_removed_rows());

    for (uint i = 0; i < n_added_cols; i++) {
      refresh_col(current_cols + i);
    }
    cache.remove_cols(msg.get_removed_cols());

  }
}