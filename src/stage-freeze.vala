/*
= Ignore events =

The freeze stage allows us to temporarily block the flow of changes along the
pipeline.  This will make it easier to implement cold and warm brushing.

*/


public class GGobi.StageFreeze : Stage {
  public bool freeze = false;
  public Matrix cache;
  
  public void freeze() {
    freeze = true;
    for(uint i = 0; i < n_rows; i++) {
      for(uint j = 0; j < n_cols; j++) {
        cache = parent.get_raw_value(i, j);
      }
    }
  }
  
  public void unfreeze() {
    freeze = false;
  }
  
  override double get_raw_value(uint i, uint j) {
    if (freeze) {
      return cache.vals[i, j];
    } else {
      return parent.get_raw_value(i, j);
    }
	}

	override void set_raw_value(uint i, uint j, double value) {
    if (!freeze) parent.set_raw_value(i, j, value);
	}

}