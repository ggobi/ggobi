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
using Math;

public class GGobi.StageImpute : Stage {
  public Imputation[] imputation;
  public Matrix cache;

  /* Rerun imputations */
  public void refresh() {
    for (uint j = 0; j < n_cols; j++) refresh_col(j);
  }
  public void refresh_col(uint j) {
    imputation[j].impute(this, j);
  }

  public override double get_raw_value(uint i, uint j) {
    return cache.vals[i,j];
  }

  public override void set_raw_value(uint i, uint j, double value) {
    cache.vals[i, j] = value;
    if (!is_missing(i, j)) parent.set_raw_value(i, j, value);
  }
}


public class GGobi.Imputation : Object {
  
  /* Return value indicates if any values were imputed */
  public bool impute(StageImpute stage, uint j) {
    if (!stage.get_variable(j).has_missings()) return(false);
    
    pre_compute(stage, j);
    
    for(uint i = 0; i < stage.n_rows; i++) {
      stage.cache.vals[i,j] = (stage.is_missing(i, j) ? stage.get_raw_value(i, j) : impute_single(i));
    }
    return true;
  }
  
  public abstract double impute_single(uint i);
  public abstract void pre_compute(StageImpute stage, uint j);
}

/* Replace missing values with a fixed value */
public class GGobi.ImputationFixed : Imputation {
  public double fixed_value = 0;
  
  double impute_single(uint i) {
    return fixed_value;
  }
}

/* Replace missing values with a column mean */
public class GGobi.ImputationMean : ImputationFixed {
  void pre_compute(StageImpute stage, uint j) {
    fixed_value = stage.get_variable(j).get_mean();
  }
}
/* Replace missing values with a column median */
public class GGobi.ImputationMedian : ImputationFixed {
  void pre_compute(StageImpute stage, uint j) {
    fixed_value = stage.get_variable(j).get_median();
  }
}

/* Impute with jittered values fixed percent above/below lowest/highest value */
public class GGobi.ImputationPercent : Imputation {
  /* Percent below/above lowest/highest value - in [-1, 1] */
  double percent = 0.15;
  double fixed_value = 0;

  /* Range of jittering within band */
  double range = 0;
  
  void pre_compute(StageImpute stage, uint j) {
    double min = stage.get_variable(j).get_min();
    double max = stage.get_variable(j).get_max();
    double range = max - min;

    double side = (percent > 0) ? max : min;
    fixed_value = side + percent * range;
    range = Math.fabs(fixed_value - side) * 0.2;
  }
  
  double impute_single(uint i) {
    return fixed_value + Random.double_range(-range, range);
  }
}

/* Impute with randomly selected non-missing value */
public class GGobi.ImputationRandom : Imputation {
  public double[] non_missing;
  
  void pre_compute(StageImpute stage, uint j) {
    non_missing.resize((int) stage.n_rows - (int) stage.get_col_n_missing(j));
    
    uint present = 0;
    for (uint i = 0; i < stage.n_rows; i++) {
      if (!stage.is_missing(i, j)) non_missing[present++] = stage.get_raw_value(i, j);
    }
  }

  double impute_single(uint i) {
    return non_missing[Random.int_range(0, non_missing.length)];
  }
}