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
    _refresh();
    for (uint j = 0; j < n_cols; j++) col_data_changed(j);    
    flush_changes_here();
  }
  private void _refresh() {
    for (uint j = 0; j < n_cols; j++) _refresh_col(j);    
  }

  public void refresh_col(uint j) {
    _refresh_col(j);
    col_data_changed(j);
    flush_changes_here();
  }
  public void _refresh_col(uint j) {
    float range = get_variable(j).get_range();
    // GLib.debug("Refreshing column %i.  Range %g", j, range);
    for (uint i = 0; i < n_rows; i++) 
      ((double[]) cache.vals[i])[j] = rand() * range;    
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
    double original;
    if (amount[j] == 0) {
      original = value;
    } else {
      original = (value - ((double[]) cache.vals[i])[j] * amount[j]) / (1 - amount[j]);
    }
    parent.set_raw_value(i, j, original);
  }  
  
  /* Process incoming change events */
  override void process_outgoing(PipelineMessage msg) {
    uint current_cols = n_cols;
    base.process_outgoing(msg);

    uint n_added_cols = msg.get_n_added_cols();
    uint n_added_rows = msg.get_n_added_rows();
    
    if (cache == null) {
      // Fresh initialisation
      cache = new Matrix(n_added_rows, n_added_cols);
      amount.resize((int) n_added_cols);
      _refresh();
    } else {
      // Update cache matrix
      cache.add_cols((int) n_added_cols);
      for (uint j = 0; j < n_added_cols; j++)
        _refresh_col(current_cols + j);
      
      if (n_added_rows > 0) {
        cache.add_rows((int) n_added_rows);
        // Need to add generate new jitters
      }
      
      cache.remove_rows(msg.get_removed_rows());
      cache.remove_cols(msg.get_removed_cols());
      
      // Resize amounts vector
      amount.resize((int) current_cols + (int) n_added_cols); 
    }
  }
}