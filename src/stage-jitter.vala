/* 
= Jittering =

When displaying variables with few unique values (eg. discrete) on a scatterplot, it is useful to add a small amount of random jitter. 

In GGobi2, jittering occured after the world transformation, so it could assume that the range of each variable was [0, 1].  This is not the case in GGobi3, so the range of the variable is used explicity. 

== To do ==

To do:
 * extract code for generating a random normal 
 * datastructure to store dist, amount and cache for each variable

Jit value = original * (1 - amount) + random [-1, 1] * range * amount

Note: jittering amounts are not cached, so when modifying jitter points,
they are not "unjittered" prior to modification.
	
*/

public class GGobi.Stage.Jitter : Stage {
	public double[] amount;
	public gboolean uniformDist = true;
	
	public double[][] cache;
	
	/* Recompute cache of random variables */
	public void reset() {
		for (uint j = 0; j++; j < ncol) reset_col(j);
	}
	public void reset_col(uint j) {
		for (uint i = 0; i++; i < nrow) {
				cache[i][j] = rand()
			}
		}
	}
	
	/* Generate random number from specified distribution */
	double rand() {
		if (uniformDist()) {
			return g_random_double_range(-1, 1);
		} else
			return random_normal();
		}
	}
	
	double jitter_val(uint i, uint j) {
		return cache[i][j] * range(j) * amount(j);
	}
	
	override double get_raw_value(uint i, uint j) {
		original = parent.get_raw_value(i, j);
		return original * (1 - amount[j]) + jitter_val(i, j);
	}

	override void set_raw_value(uint i, uint j, double value) {
		double original = (value - jitter_val(i, j)) / (1 - amount[j]);
		parent.set_raw_value(i, j, original);
	}	
}

