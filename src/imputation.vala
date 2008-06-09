/* Imputation classes

Basic imputation classes used by StageImpute */

using GLib;

public abstract class GGobi.Imputation : Object {
  public void impute(StageImpute stage, uint j) {
    pre_compute(stage, j);
    
    for(uint i = 0; i < stage.n_rows; i++) {
      double value = !stage.is_missing(i, j) ? stage.parent.get_raw_value(i, j) : impute_single(i);
      stage.cache.set(i, j, value);
    }
  }
  
  public abstract double impute_single(uint i);
  public virtual void pre_compute(StageImpute stage, uint j) { }
  public abstract string description();
  public virtual bool equals(Imputation that) {
    return that.get_type() == that.get_type();
  }
}

/* Replace missing values with a fixed value */
public class GGobi.ImputationFixed : Imputation {
  public double fixed_value = 0;
  
  override double impute_single(uint i) {
    return fixed_value;
  }
  
  override string description() {
    return fixed_value.to_string("Value: %.2f");
  }
  
  override bool equals(Imputation that) {
    if (this.get_type() != that.get_type()) return(false);
    
    return ((ImputationFixed) that).fixed_value == fixed_value;
  }
  
}

/* Replace missing values with a column mean */
public class GGobi.ImputationMean : ImputationFixed {
  override void pre_compute(StageImpute stage, uint j) {
    fixed_value = stage.get_variable(j).get_mean();
  }
  override string description() {
    return fixed_value.to_string("Mean: %.2f");
  }
}
/* Replace missing values with a column median */
public class GGobi.ImputationMedian : ImputationFixed {
  override void pre_compute(StageImpute stage, uint j) {
    fixed_value = stage.get_variable(j).get_median();
  }
  override string description() {
    return fixed_value.to_string("Median: %.2f");
  }
}

/* Impute with jittered values fixed percent above/below lowest/highest value */
public class GGobi.ImputationPercent : Imputation {
  /* Percent below/above lowest/highest value - in [-1, 1] */
  public double percent = -0.15;
  double fixed_value = 0;

  /* Range of jittering within band */
  double jitter = 0;
  
  override void pre_compute(StageImpute stage, uint j) {
    double min = stage.get_variable(j).get_min();
    double max = stage.get_variable(j).get_max();
    double range = max - min;

    if (range < EPSILON) {
      fixed_value = 0;
      jitter = 0.2;
    } else {
      double side = (percent > 0) ? max : min;
      fixed_value = side + percent * range;
      jitter = percent * range * 0.2;      
    }

  }
  
  override double impute_single(uint i) {
    return fixed_value + Random.double_range(-jitter, jitter);
  }
  override string description() {
    return percent.to_string("Percent: %.2f");
  }
  
  override bool equals(Imputation that) {
    if (this.get_type() != that.get_type()) return(false);
    
    return ((ImputationPercent) that).percent == percent;
  }
  
}

/* Impute with randomly selected non-missing value */
public class GGobi.ImputationRandom : Imputation {
  public double[] non_missing;
  
  override void pre_compute(StageImpute stage, uint j) {
    non_missing.resize((int) stage.n_rows - (int) stage.get_col_n_missing(j));
    
    uint present = 0;
    for (uint i = 0; i < stage.n_rows; i++) {
      if (!stage.is_missing(i, j)) non_missing[present++] = stage.parent.get_raw_value(i, j);
    }
  }

  override double impute_single(uint i) {
    if (non_missing.length == 0) return(0);
    
    return non_missing[Random.int_range(0, non_missing.length)];
  }
  override string description() {
    return "Random";
  }
  
}