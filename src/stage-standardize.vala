/* 
== Comparable ranges ==
(`tform_to_world`)

In the absence of information to the contrary, it is important to ensure that all
variables are on a common scale. In GGobi2 this involves rescaling each variable to [0,
1] (`tform_to_world`). However, there are some cases when that is incorrect, for example
if we know some variables were measured on the same scale.

By default, all variables are scaled with f(x) = x - min(x) / (max(x) - min(x)), however, it needs to be possible to override min and max with values set by the user, rather than computed from the data.

*/
public class GGobi.Stage.Standardize : Stage {
	
	double standardize(double value, min, max) {
		value - min / (min - max)
	}
	
	double unstandardize(double value, min, max) {
		value * (min - max) + min
	}
	
	override double get_raw_value(uint i, uint j) {
		var = get_variable(j)
		standardize(parent.get_raw_value(i, j), var.min, var.max)
	}

	override void set_raw_value(uint i, uint j, double value) {
		var = get_variable(j)
		parent.set_raw_value(i, j, unstandardise(value, var.min, var.max))
	}
}