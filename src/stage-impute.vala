/* 
= Imputation =

Imputation is used to replace missing values with plausible numbers.  This is particularly important for displays such as the tour, as there is no other reasonable way to deal with missing values apart from using complete cases. 

Each variable has a imputation type, and various parameters to that type.
Almost identical to transformation, but random (condition on symbol and colour)
so needs more information.  In general, any imputation for a single column might
use might use information Could the stage be passed as a parameter to the 
transformation function?

There are currently:

	* fixed (user specified, mean, median, % below/above lowest/highest value)
	* random

And can be applied either the whole data set, or per symbol/colour group

== To do ==

 * convert each imputation type into its own object
	
*/

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
		values[i][j]
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
    
    
  }
	
}

public class GGobi.Imputation.Fixed : Imputation {
  double fixed_value = 0;
	
	
	double impute_single() {
    return fixed_value;
	}
	
}

public class GGobi.Imputation.Percent : Fixed {
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
    range = abs(fixed_value - side) * 0.2;
  }
  
  double impute_single() {
    return fixed_value + g_random_double_range(-1, 1) * range;
  }
	
}

public class GGobi.Imputation.Mean : Imputation.Fixed {
  
	
}
public class GGobi.Imputation.Median : Imputation.Fixed {
	
}

public class GGobi.Imputation.Random : Imputation {
	
}