/* 
= Imputation =

Imputation is used to replace missing values with plausible numbers. This is
particularly important for displays such as the tour, as there is no other
reasonable way to deal with missing values apart from using complete cases.

Each variable has a imputation type, and various parameters to that type.
Almost identical to transformation, but random (condition on symbol and
colour) so needs more information. In general, any imputation for a single
column might use might use information Could the stage be passed as a
parameter to the transformation function?

GGobi3 misses the more complicated imputation schemes of GGobi2 (eg.
seperately by group). The assumptions that if the user wants something more
complicated, they can implement it themselves in R.

There are currently:

  * fixed (user specified, mean, median, % below/above lowest/highest value)
  * random

And can be applied either the whole data set, or per symbol/colour group

== To do ==

*/

using GLib;

public class GGobi.StageImpute : Stage {
  public Imputation[] imputation;
  public Matrix cache;
  
  public signal void imputation_changed(uint j);

  /* Rerun imputations */
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
    imputation[j].impute(this, j);
  }
  
  public override double get_raw_value(uint i, uint j) {
    return ((double[]) cache.vals[i])[j];
  }

  public override void set_raw_value(uint i, uint j, double value) {
    ((double[]) cache.vals[i])[j] = value;
    if (!is_missing(i, j)) parent.set_raw_value(i, j, value);
  }
  
  override void process_outgoing(PipelineMessage msg) {
    uint current_cols = n_cols;
    base.process_outgoing(msg);

    uint n_added_cols = msg.get_n_added_cols();
    uint n_added_rows = msg.get_n_added_rows();
    
    if (cache == null) {
      // Fresh initialisation
      cache = new Matrix(n_added_rows, n_added_cols);
      imputation.resize((int) n_added_cols);
      for (int j = 0; j < n_added_cols; j++)
        imputation[j] = new ImputationPercent();
      
      _refresh();
    } else {
      // Need stage+matrix method that wraps all of this up
      imputation.resize((int) n_added_cols);
      
      // Update cache matrix
      cache.add_cols((int) n_added_cols);
      for (uint j = 0; j < n_added_cols; j++)
        _refresh_col(current_cols + j);
      
      if (n_added_rows > 0) {
        cache.add_rows((int) n_added_rows);
      }
      
      cache.remove_rows(msg.get_removed_rows());
      cache.remove_cols(msg.get_removed_cols());
    }
  }
  
  // Need set imputation method which calls impute
  
}


