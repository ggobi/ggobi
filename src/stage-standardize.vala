/* 
== Comparable ranges ==
(`tform_to_world`)

In the absence of information to the contrary, it is important to ensure that
all variables are on a common scale. In GGobi2 this involves rescaling each
variable to [0, 1] (`tform_to_world`). However, there are some cases when that
is incorrect, for example if we know some variables were measured on the same
scale.

By default, all variables are scaled with f(x) = x - min(x) / (max(x) -
min(x)), however, it needs to be possible to override min and max with values
set by the user, rather than computed from the data.

*/

public class GGobi.StageStandardize : Stage {
  double standardize(double value, double min, double max) {
    if ((max - min) < EPSILON) return 0.5;
    return (value - min) / (max - min);
  }
  
  double unstandardize(double value, double min, double max) {
    if ((max - min) < EPSILON) return min;
    return value * (max - min) + min;
  }
  
  override double get_raw_value(uint i, uint j) {
    Variable v = get_variable(j);
    if (v.is_attribute) return(parent.get_raw_value(i, j));
    
    return standardize(parent.get_raw_value(i, j), v.get_min(), v.get_max());
  }

  override void set_raw_value(uint i, uint j, double value) {
    Variable v = get_variable(j);
    if (v.is_attribute) {
      parent.set_raw_value(i, j, value);
    } else {
      parent.set_raw_value(i, j, unstandardize(value, v.get_min(), v.get_max()));      
    }
  }
}