/* Describes changes in the pipeline data matrix. It communicates five different
   types of changes:
   
   1) Remove columns
   2) Remov rows
   3) Change data (indexed by column)
   4) Add columns
   5) Add rows

   Note that insertion is not supported; all addition occur at the ends of the
   matrix. Also there is no event for row changes, because it is redundant with
   column changes, which occur more often and are more efficiently handled by
   embedding environments, since there are usually less columns than rows.

   These changes obviously conflict with each other, so the order in which they
   are considered is important. Changes occur in the following order: deletion,
   update, addition. The reasoning for this is thus: it is wasteful to add or
   change elements that will soon be deleted and we also want to avoid changing
   elements that are still to be added.

   In the case of columns, this means that the indices of the changed columns
   take into account any removals. It also means that it is not possible to
   remove or change indices that have been appended, since appending takes
   place after removal and changing.

   When a message is received, usually from the parent staage, its indexing 
   should be compatible with that of the receiver. Some pipeline stages, 
   specifically filters, change the indexing by removing rows and/or columns. 
   It is the responsibility of those stages to use the filter_rows() and
   filter_cols() convenience methods to modify the indexing by providing the
   indices of the hidden rows and columns, respectively. Filtering blocks
   all types of events: removals, changes, and additions.
   
   Some stages, which are extensions of filters, essentially replace the data
   matrix by column (like in tours) or row or both (like generating a dataset
   describing the variables, ie an experimental design matrix). In the first
   two cases, it may be simpler to allow resetting the message by row or
   column. This means clearing out all previous events and resetting the
   initial dimension. This is achieved by the reset_*() functions. 
   Redefining both rows and columns warrants the generation of a new message.
*/

using GLib;

public class GGobi.PipelineMessage : Object  {
  /* The Bitset is used essentially as a fast set of indices */
  private Bitset changed_cols;
  private Bitset removed_cols;
  private Bitset removed_rows;
  
  construct {
    changed_cols = new Bitset();
    changed_cols.size = n_cols;
    removed_cols = new Bitset();
    removed_cols.size = n_cols;
    removed_rows = new Bitset();    
    removed_rows.size = n_rows;
  }
  
  /* since we only append (but not insert) rows/cols, the counts are enough */
  private uint n_added_rows;
  private uint n_added_cols;

  /* n_cols and n_rows are now writeable properties, because it is convenient
     to change them when redefining the matrix in terms of rows or cols (tours).
  */
  private uint _n_cols;
  public uint n_cols {
    get { return _n_cols; }
    set {
      _n_cols = value;
      changed_cols.size = _n_cols;
      removed_cols.size = _n_cols;
    }
  }     

  private uint _n_rows;
  public uint n_rows {
    get { return _n_rows; }
    set {
      _n_rows = value;
      removed_rows.size = _n_rows;
    }
  }     

  public PipelineMessage(uint n_rows, uint n_cols) {
    this.n_rows = n_rows;
    this.n_cols = n_cols;
  }
  
  // Lists the indices of the changed columns.
  public SList<uint>? get_changed_cols() {
    return changed_cols.get_indices();
  }
  // Lists the indices of the removed columns
  public SList<uint>? get_removed_cols() {
    return removed_cols.get_indices();
  }
  // Lists the indices of the removed columns
  public SList<uint>? get_removed_rows() {
    return removed_rows.get_indices();
  }
  
  // maybe it would be better to also give ranges here just for convenience?   

  public uint get_n_added_cols() {
    return n_added_cols;
  }
  public uint get_n_added_rows() {
    return n_added_rows;
  }
  public uint get_n_removed_rows() {
    return removed_rows.get_n_indices();
  }
  public uint get_n_changed_cols() {
    return changed_cols.get_n_indices();
  }
  public uint get_n_removed_cols() {
    return removed_cols.get_n_indices();
  }
  
  /* convenience functions for iterating over changes */
  
  public void changed_cols_apply(IndexFunc func, void* data) {
    changed_cols.apply(func, data);
  }
  public void changed_cols_apply_decreasing(IndexFunc func, void* data) {
    changed_cols.apply_decreasing(func, data);
  }
  public void removed_cols_apply(IndexFunc func, void* data) {
    removed_cols.apply(func, data);
  }
  public void removed_cols_apply_decreasing(IndexFunc func, void* data) {
    removed_cols.apply_decreasing(func, data);
  }
  public void removed_rows_apply(IndexFunc func, void* data) {
    removed_rows.apply(func, data);
  }
  public void removed_rows_apply_decreasing(IndexFunc func, void* data)   {
    removed_rows.apply_decreasing(func, data);
  }
  public void added_rows_apply(IndexFunc func, void* data)
  {
    for (uint j = n_rows; j < n_rows + n_added_rows; j++)
      func(j, data);
  }
  public void added_cols_apply(IndexFunc func, void* data) {
    for (uint j = n_cols; j < n_cols + n_added_cols; j++)
      func(j, data);
  }
  
  /**
   * change_col:
   * @self: a #GGobiPipelineMessage
   * @j: index of the column that changed
   *
   * Registers a change to a specified column. If the column is in the
   * appended set, this has no effect.
   * Note: unlike the events that change the dimensions, this one takes
   * a single element, not a list
   *
   */
  public void change_col(uint j)  {
    // g_return_if_fail(j < n_cols + n_added_cols);
    if (j < n_cols)
      changed_cols.set_bit(j);
  }
  
  // Registers a change to a list of columns. 
  public void change_cols(SList<uint>? cols) {
    foreach(uint j in cols) 
      change_col(j);
  }
  
  // Registers the removal of a list of a columns. Indices of changed columns
  // are automatically shifted, because they are assumed to occur after removal.
  public void remove_cols(SList<uint>? cols) {
    /* ensure sorted for index updating */
    SList<uint> sorted_cols = cols.copy();
    sorted_cols.sort(PipelineMessage.index_compare);
    
    foreach(uint index in cols) {
      // g_return_if_fail(index < n_cols + n_added_cols);
      if (index < n_cols)
        removed_cols.set_bit(index);
    }
    changed_cols.remove_indices(sorted_cols);
  }

  // Registers the removal of a list of rows.
  public void remove_rows(SList<uint>? rows) {
    foreach(uint index in rows) {
      // g_return_if_fail(index < n_rows + n_added_rows);
      if (index < n_cols)
        removed_rows.set_bit(index);
    }
  }
  
  // Registers the appending of @n columns.
  public void add_cols(uint n) {
    n_added_cols += n;
  }
  // Registers the appending of @n rows.
  public void add_rows(uint n) {
    n_added_rows += n;
  }
  
  /* filter changed_cols and removed_cols */
  
  /**
   * This adjusts the message as if the given columns in the original matrix
   * never existed. The indices of changed columns and removed columns are
   * shifted accordingly. This is useful when passing a message through
   * a filter of the data matrix.
   *
   */
  public void filter_cols(SList<uint>? cols) {
    SList<uint> sorted_cols = cols.copy();
    sorted_cols.sort(index_compare);
    uint new_cols = n_cols;
    
    foreach(uint i in cols) {
      if (i >= new_cols) {
        n_added_cols--;
      } else {
        new_cols--;
      }
    }
    n_cols = new_cols;
    changed_cols.remove_indices(sorted_cols);
    removed_cols.remove_indices(sorted_cols);
  }

  /**

   * This adjusts the message as if the given rows in the original matrix
   * never existed. The indices of removed rows are
   * shifted accordingly. This is useful when passing a message through
   * a filter of the data matrix.
   *
   */
  public void filter_rows(SList<uint>? rows) {
    SList<uint> sorted_rows = rows.copy();
    sorted_rows.sort(index_compare);

    uint new_rows = n_rows;
    
    foreach(uint i in rows) {
      if (i >= new_rows) {
        n_added_rows--;
      } else {
        new_rows--;
      }
    }
    n_rows = new_rows;
    removed_rows.remove_indices(sorted_rows);
    // g_signal_emit_by_name("notify::last_row");
  }
  
  /* consuming events */
  
  /**
   * Erases all row events. This is usually followed by registration of new
   * row events on different (sorted, shifted, etc) indices. 
   * Useful when the rows of the data matrix are completely rearranged.
   */
  public void consume_rows() {
    removed_rows.clear();
    n_added_rows = 0;
  }
  
  /**
   * Erases all columns events. This is usually followed by registration of new column
   * events on different (sorted, shifted, etc) indices. 
   * Useful when the columns of the data matrix are completely rearranged.
   */
  public void consume_cols() {
    removed_cols.clear();
    changed_cols.clear();
    n_added_cols = 0;
  }
  
  /**
   * Translates the events in @other into this message. @other is 
   * assumed to contain changes that occurred AFTER the changes in this message.
   * This is important when removal events have occurred in this message.
   */
  public void merge(PipelineMessage other) {
    uint over;
    SList<uint> elements, elements_shifted;
    
    elements_shifted = shift_indices(removed_rows, other.removed_rows, 
      out over);
    remove_rows(elements_shifted);
    n_added_rows -= over;
    
    elements_shifted = shift_indices(removed_cols, other.removed_cols, 
      out over);
    remove_cols(elements_shifted);
    n_added_cols -= over;
    
    elements = other.get_changed_cols();
    change_cols(elements);
    
    add_rows(other.get_n_added_rows());
    add_cols(other.get_n_added_cols());
  }
  
  /* utilities */
  
  private SList<uint>? shift_indices(
    Bitset first, Bitset second, out uint over
  ) {
    SList<uint> first_ind = first.get_indices(), shifted, ind;
    uint my_size = first.size;

    Bitset secondc = second.clone();
    secondc.size = my_size;
    secondc.insert_indices(first_ind);

    shifted = secondc.get_indices();

    over = 0;
    foreach(uint index in first_ind) {
      if (index >= my_size) over++;
    }

    return shifted;
  }
  
  public static int index_compare(void* a, void* b) {
    return (int)a - (int)b;
  }
}
