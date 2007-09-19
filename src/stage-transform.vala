
// FIXME: Add names transformation back in

// static string name_tform_func(string name, pointer data)
// {
//   Transform tform = GGOBI_TRANSFORM(data);
//   return(ggobi_transform_variable_name(tform, name));
// }

/* This stage caches every value, even those it does not transform */

using GLib;

public class GGobi.StageTransform : Stage {
  private Matrix tform; 
  private HashTable<uint, Transform> active_tforms;
  
  private signal void applied(uint j, Transform tform);

  public StageTransform(construct Stage parent) {}
  
  construct {
    tform = new Matrix(0, 0);
    active_tforms = new HashTable.full(null, null, null, g_object_unref);
  }
  
  override void process_outgoing(PipelineMessage msg)  {
    SList changed_cols = msg.get_changed_cols();
    SList removed_rows = msg.get_removed_rows();
    SList removed_cols = msg.get_removed_cols();
    uint n_added_cols = msg.get_n_added_cols();
    uint n_added_rows = msg.get_n_added_rows();
    uint n_refresh = n_added_cols;
    
    parent.process_outgoing(msg);
    
    tform.remove_rows(removed_rows);
    tform.add_rows((int) n_rows);
    tform.remove_cols(removed_cols);
    tform.add_cols((int) n_cols);
    
    // FIXME: Need to update hash table in response to column deletions
    
    if (removed_rows != null || n_added_rows > 0) {
      // if rows changed, refresh all transforms
      n_refresh = n_cols; 
    } else {
      foreach(uint j in changed_cols) transform(0, j);
    }
    for (uint j = n_cols - n_refresh; j < n_cols; j++)
      transform(0, j);

    /* don't refresh if no columns (reparenting) */
    /* this is a temporary hack until the pipeline and displays are completed */
    if (gg != null && n_cols > 0) displays_tailpipe (RedrawStyle.FULL, gg);
  }
    
  /**
   * Applies the transform to the specified column. The transform is
   * persistent, so when values in the underlying data change, they are
   * transformed.  It is only possible to register a single transform 
   * against a given column, per transform stage.
   */
  public void apply(uint j, Transform tform) {
    Variable v = get_variable(j);
    if (tform != null) {
      // v.set_name_transform_func(null, null);
      active_tforms.remove(j);
    } else {
      tform.notify += tform => {
        transform_notify_cb(tform);
      };
      // v.set_name_transform_func(name_tform_func, tform);
      active_tforms.insert(j, tform);
    }
    transform(0, j);
    flush_changes_here();
    // temporary hack
    displays_tailpipe(RedrawStyle.FULL, gg);
    applied(j, tform);
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
    return ((double[]) tform.vals[i])[j];
  }
  
  override void set_raw_value(uint i, uint j, double value) {
    Transform tf = get_transform(j);
    
    if (tform != null) {
      Variable v = parent.get_variable(j);
      
      double[] input = new double[1]; 
      input[0] = value;
      double[] res = tf.reverse(input, v);
      if (res != null) {
        value = res[0];
      } else {
        transform_error(tf, j);
        return;
      }
    }
    base.set_raw_value(i, j, value);
    ((double[]) tform.vals[i])[j] = value;
  }
  
  private void transform(uint start, uint j) {
    uint i;
    Transform tf = get_transform(j);
    if (tform != null) { 
      // identity transformation
      for (i = 0; i < n_rows; i++)
        ((double[]) tform.vals[i])[j] = parent.get_raw_value(i, j);
    } else {
      double[] result = tf.column(parent, start, j); 
      if (result != null) {
        for (i = start; i < n_rows; i++)
          ((double[]) tform.vals[i])[j] = result[i];
      } else {
        transform_error(tf, j);
      } 
    }
    // FIXME: world stage should take care of this
    tform_to_world_by_var(this, j);
    col_data_changed(j);
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
      if (get_transform(col) == tf) transform(0, col);
    }
    flush_changes_here();
    // temporary hack
    displays_tailpipe (RedrawStyle.FULL, gg);
  }
}
