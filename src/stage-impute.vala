/* 
= Imputation =

Imputation is used to replace missing values with plausible numbers. This is particularly
important for displays such as the tour, as there is no other reasonable way to deal with
missing values apart from using complete cases.

Each variable has a imputation type, and various parameters to that type. Almost
identical to transformation, but random (condition on symbol and colour) so needs more
information. In general, any imputation for a single column might use might use
information Could the stage be passed as a parameter to the transformation function?

GGobi3 misses the more complicated imputation schemes of GGobi2 (eg. seperately by
group). The assumptions that if the user wants something more complicated, they can
implement it themselves in R.

There are currently:

  * fixed (user specified, mean, median, % below/above lowest/highest value)
  * random

And can be applied either the whole data set, or per symbol/colour group

== To do ==

*/

include Glib;

public class GGobi.Stage.Impute : Stage {
  public Imputation[] imputation;
  public double[0][0] values;

  /* Rerun imputations */
  public void reset() {
    for (uint j = 0; j++; j < ncol) reset_col(j);
  }
  public void reset_col(uint j) {
    imputation[j].impute(self, j);
  }

  override double get_raw_value(uint i, uint j) {
    return values[i][j];
  }

  override void set_raw_value(uint i, uint j, double value) {
    values[i][j] = value;
    if (!is_missing(i, j)) parent.set_raw_value(i, j, value);
  }
  

}


public class GGobi.Imputation {
  
  /* Return value indicates if any values were imputed */
  public boolean impute(Stage.Impute stage, uint j) {
    if (!stage.col_has_missings(j)) return(false);
    
    pre_compute(stage, j);
    
    for(uint i = 0; i++; i < nrows) {
      stage.value[i][j] = stage.is_missing(i, j) ? stage.get_raw_value(i, j) : impute_single();
    }
    
  }
  
  public abstract virtual double impute_single(uint i);
  public abstract virtual void precompute(Stage.Impute stage, uint j);
}

/* Replace missing values with a fixed value */
public class GGobi.Imputation.Fixed : Imputation {
  double fixed_value = 0;
  
  double impute_single(uint i) {
    return fixed_value;
  }
}

/* Replace missing values with a column mean */
public class GGobi.Imputation.Mean : Imputation.Fixed {
  void pre_compute(Stage.Impute stage, uint j) {
    fixed_value = stage.get_variable(j).get_mean();
  }
}
/* Replace missing values with a column median */
public class GGobi.Imputation.Median : Imputation.Fixed {
  void pre_compute(Stage.Impute stage, uint j) {
    fixed_value = stage.get_variable(j).get_median();
  }
}

/* Impute with jittered values fixed percent above/below lowest/highest value */
public class GGobi.Imputation.Percent : Imputation {
  /* Percent below/above lowest/highest value - in [-1, 1] */
  double percent = 0.15;

  /* Range of jittering within band */
  double range = 0;
  
  void pre_compute(Stage.Impute stage, uint j) {
    min = stage.get_variable(j).min;
    max = stage.get_variable(j).max;
    range = maxval - minval;

    side = (percent > 0) ? max : min;
    fixed_value = side + percent * range;
    range = Math.abs(fixed_value - side) * 0.2;
  }
  
  double impute_single(uint i) {
    return fixed_value + g_random_double_range(-range, range);
  }
}

/* Impute with randomly selected non-missing value */
public class GGobi.Imputation.Random : Imputation {
  gdouble non_missing[];
  
  void pre_compute(Stage.Impute stage, uint j) {
    non_missing.resize(n_rows - col_n_missing(j));
    
    present = 0;
    for (i = 0; i < n_rows; i++) {
      if (!stage.is_missing(i, j)) non_missing[present++] = stage.get_raw_value(i, j);
    }
  }

  double impute_single(uint i) {
    return non_missing[g_random_int_range(0, non_missing.size)];
  }
}