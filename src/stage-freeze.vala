/*
= Ignore events =

The freeze stage allows us to temporarily block the flow of changes along the
pipeline.  This will make it easier to implement cold and warm brushing.

While frozen, all incoming events are ignored.  While unfrozen, everything
is proxied directly to the parent stage.  Freezing saves a cache of the 
current values of the parent stage.

TODO: ensure variables are frozen as well.

*/


public class GGobi.StageFreeze : Stage {
  private bool _frozen = false;
  private bool _dirty = false;
  private PipelineMatrix cache;
  
  construct {
    cache = new PipelineMatrix();
  }  
  
  public void freeze() {
    _frozen = true;
    _dirty = false;

    cache.add_rows(parent.n_rows);
    cache.add_cols(parent.n_cols);
    refresh();
  }
  
  public void unfreeze() {
    _frozen = false;
    if (!is_dirty()) return;
    
    // If dirty, send change events
    for(uint j = 0; j < parent.n_cols; j++) {
      col_data_changed(j);
    }
    flush_changes_here();
    _dirty = false;
  }
  
  public bool is_dirty() {
    return _dirty;
  }
  
  public override void refresh_col_(uint j) {
    for(uint i = 0; i < n_rows; i++) {
      cache.set(i, j, parent.get_raw_value(i, j));
    }
  }
  
  public override double get_raw_value(uint i, uint j) {
    if (_frozen) {
      return cache.get(i, j);
    } else {
      return parent.get_raw_value(i, j);
    }
  }
  
  public override void set_raw_value(uint i, uint j, double value) {
    if (_frozen) {
      cache.set(i, j, value);
    } else {
      parent.set_raw_value(i, j, value);
    }
  }

  public override void process_outgoing(PipelineMessage msg) {
    if (_frozen) {
      _dirty = true;
    } else {
      base.process_outgoing(msg);
    }
  }
}