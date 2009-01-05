/*
 A simple filter based on a specified boolean column in the data matrix.
 For custom filters, insert a stage before this one that modifies
 the filter column.
*/
using GLib;

public class GGobi.StageFilter : Stage {
  public int filter_col { get; construct set; }

  /* filtered index => raw index */
  public uint[] included_rows;
  /* raw index => filtered index, -1 = filtered */
  public int[] included_rows_rev;
  /* whether a (raw) row passes the filter */
  private bool[] included;

  construct {
    filter_col = -1;
  }

  // Returns: %TRUE if the parent row is allowed past the filter
  public bool is_included(uint i) {
    return included[i];
  }
  
  private bool is_included_raw(uint i) {
    return (filter_col == -1) ? true : 
      (bool) parent.get_raw_value(i, filter_col);
  }
  
  // This is a convenience function that sets the cell at row @i in the
  // filter column to @val in the parent stage.
  public void set_included(uint i, bool val) {
    parent.set_raw_value(i, filter_col, (double) val);
  }

  // Convenience function for setting the entire filter column at once.
  public void set_included_all(bool value) {
    for (uint i = 0; i < parent.n_rows; i++)
      set_included(i, value);
  }

  // Override accessor methods to get unfiltered value
  
  public override void set_missing(uint i, uint j) {
    parent.set_missing(included_rows[i], j);
  }

  public override bool is_missing(uint i, uint j) {
    return parent.is_missing(included_rows[i], j);  
  }

  public override string get_row_id(uint i) {
    return parent.get_row_id(included_rows[i]);  
  }

  public override int get_row_for_id(string id) {
    int row = parent.get_row_for_id(id);
    if (row == -1)
      return -1;
    return included_rows_rev[row];
  }

  public override void set_raw_value(uint i, uint j, double value) {
    parent.set_raw_value(included_rows[i], j, value);
  }

  public override double get_raw_value(uint i, uint j) {
    return parent.get_raw_value(included_rows[i], j);
  }
  
  public override void process_incoming(PipelineMessage msg) {
    SList<uint> changed_cols = msg.get_changed_cols();
    uint rows_changed = msg.get_n_added_rows() + 
      msg.get_n_removed_rows();
    msg.consume_rows(); // we ignore any row changes 
    // FIXME: need to check whether filter column has been shifted or removed
    /* rows added, rows removed, or filter column changed: update filter */
    if ((rows_changed > 0) || changed_cols.find(filter_col) != null)
      refresh_();
    base.process_incoming(msg);
  }

  // Update the included state of the rows, sending messages as necessary
  public void refresh_() {
    int n_rows = parent != null ? (int) parent.n_rows : 0;
    int n_included = 0;
    int n_included_prev = included.length;

    SList<uint> removed_rows = new SList<uint>();

    // Guarantee big enough
    included_rows.resize(n_rows);
    included_rows_rev.resize(n_rows);
    included.resize(n_rows);

    for (int i = 0; i < n_rows; i++) {
      included[i] = is_included_raw((uint) i);

      if (included[i]) {
        included_rows_rev[i] = n_included;
        included_rows[n_included++] = i;        
      }
    }

    // Shrink to correct size
    included_rows.resize(n_included);
    included_rows_rev.resize(n_included);
    included.resize(n_included);
    
    if (n_included_prev > n_included) {
      // GLib.debug("More old than new: %i -> %i", n_included_prev, n_included);
      for(int i = n_included; i < n_included_prev; i++) {
        removed_rows.prepend(i);
      }
      rows_removed(removed_rows);

    } else if (n_included > n_included_prev) {
      // GLib.debug("More new than old: %i -> %i", n_included_prev, n_included);

      rows_added(n_rows - n_included_prev);      
    } else {
    }
  }
}
