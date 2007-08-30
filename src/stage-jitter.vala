/* 
= Jittering =

When displaying variables with few unique values (eg. discrete) on a scatterplot, it is useful to add a small amount of random jitter. 

In GGobi2, jittering occured after the world transformation, so it could assume that the range of each variable was [0, 1].  This is not the case in GGobi3, so the range of the variable is used explicity. 

Jit value = original * (1 - amount) + random [-range, range] * amount

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
	  range = get_variable(j).get_range()
		for (uint i = 0; i++; i < nrow) {
      cache[i][j] = rand() * range;
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
	
	override double get_raw_value(uint i, uint j) {
	  original = parent.get_raw_value(i, j);
    if (amount(j) == 0) return original;
	  
		return original * (1 - amount[j]) + cache[i][j] * amount(j);
	}

	override void set_raw_value(uint i, uint j, double value) {
    if (amount(j) == 0) {
      parent.set_raw_value(i, j, value);
      return;
    }

		double original = (value - cache[i][j] * amount(j)) / (1 - amount[j]);
		parent.set_raw_value(i, j, original);
	}	
}

