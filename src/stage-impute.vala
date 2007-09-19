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
  public PipelineMatrix cache;
  
  public override void refresh_col_(uint j) {
    imputation[j].impute(this, j);
  }
  
  public override double get_raw_value(uint i, uint j) {
    return cache.get(i, j);
  }

  public override void set_raw_value(uint i, uint j, double value) {
    cache.set(i, j, value);
    if (!is_missing(i, j)) parent.set_raw_value(i, j, value);
  }
  
  public void set_imputation(uint j, Imputation imp) {
    imputation[j] = imp;
    col_parameter_changed(j);
    col_data_changed(j);
    flush_changes_here();
  }
  
  construct {
    cache = new PipelineMatrix();
    imputation.resize(0);
  }
  
  override void process_outgoing(PipelineMessage msg) {
    uint old_cols = n_cols;
    base.process_outgoing(msg);

    // FIXME: deal with deleted variables too
    imputation.resize((int) n_cols);
    for (uint j = old_cols; j < n_cols; j++)
      imputation[(int) j] = new ImputationPercent();

    // needs to be done second as relies on imputations being set up
    cache.process_message(msg, this);
    
  }
  
  // Need set imputation method which calls impute
  
}


