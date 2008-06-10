
// FIXME: Add names transformation back in

// static string name_tform_func(string name, void* data)
// {
//   Transform tform = GGOBI_TRANSFORM(data);
//   return(ggobi_transform_variable_name(tform, name));
// }

/* This stage caches every value, even those it does not transform */

using GLib;

public class GGobi.StageTransform : Stage {
  private PipelineMatrix cache; 
  private HashTable<uint, Transform> active_tforms;
  
  public StageTransform(Stage parent) {
    this.parent = parent;
  }
  
  construct {
    cache = new PipelineMatrix();
    active_tforms =
      new HashTable<uint, Transform>.full(direct_hash, direct_equal, null,
                                          g_object_unref);
  }
  
  override void process_outgoing(PipelineMessage msg)  {
    // FIXME: Need to update hash table in response to column deletions 
    base.process_outgoing(msg);
    cache.process_message(msg, this);
  }
    
  /**
   * Applies the transform to the specified column. The transform is
   * persistent, so when values in the underlying data change, they are
   * transformed.  It is only possible to register a single transform 
   * against a given column, per transform stage.
   */
  public void apply(uint j, Transform? tform) {
    Variable v = get_variable(j);
    if (tform == null) {
      // v.set_name_transform_func(null, null);
      active_tforms.remove(j);
    } else {
      tform.notify += tform => {
        transform_notify_cb(tform);
      };
      // v.set_name_transform_func(name_tform_func, tform);
      active_tforms.insert(j, tform);
    }
    refresh_col(j);
    col_parameter_changed(j);
  }

  /* Remove the transform at the given column */
  public void cancel(uint j) {
    apply(j, null);
  }
  /* Cancel transforms at all columns. */
  public void cancel_all() {
    foreach(uint col in active_tforms.get_keys()) cancel(col);
  }
  
  /* #Transform for column @j */
  public Transform get_transform(uint j) {
    return active_tforms.lookup(j);
  }
  
  /* The number of columns with transforms applied. */
  public uint get_n_transformed_cols() {
    return active_tforms.get_keys().length();
  }
  
  override double get_raw_value(uint i, uint j) {
    return cache.get(i, j);
  }
  
  override void set_raw_value(uint i, uint j, double value) {
    Transform tform = get_transform(j);
    
    if (tform != null) {
      Variable v = parent.get_variable(j);
      
      double[] input = new double[1]; 
      input[0] = value;
      double[] res = tform.reverse(input, v);
      if (res != null) {
        value = res[0];
      } else {
        transform_error(tform, j);
        return;
      }
    }
    base.set_raw_value(i, j, value);
    cache.set(i, j, value);
  }
  
  public override void refresh_col_(uint j) {
    transform(0, j);
  }
  
  private void transform(uint start, uint j) {
    uint i;
    Transform tform = get_transform(j);
    if (tform == null) { 
      // identity transformation
      for (i = 0; i < n_rows; i++)
        cache.set(i, j, parent.get_raw_value(i, j));
    } else {
      double[] result = tform.column(parent, start, j); 
      if (result != null) {
        for (i = start; i < n_rows; i++)
          cache.set(i, j, result[i]);
      } else {
        transform_error(tform, j);
      } 
    }
  }
  
  private void transform_error(Transform tform, uint j)
  {
     string name = tform.get_name();
     GLib.critical("Data outside the domain of transform %s at col %d.", name, j);
  }
  
  /* whenever a property of the transform changes, assume we need to update */
  // FIXME: This may be too aggressive...
  private void transform_notify_cb(Transform tf) {
    foreach(uint col in active_tforms.get_keys()) {
      if (get_transform(col) == tf) refresh_col(col);
    }
  }
}
