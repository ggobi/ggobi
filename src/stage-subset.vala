/*
 Performs filtering operations specific to the GGobi subset dialog.
 Right now the dialog acts solely as a controller, not a view, of the filter,
 so there is no need to store the high-level rules that it applies.
*/
using GLib;

namespace GGobi {
  public enum MatchMethod {
    IDENTICAL,
    INCLUDES,
    STARTS_WITH,
    ENDS_WITH,
    EXCLUDES
  } 
}

public class GGobi.StageSubset : StageFilter  {

  // Update the filter, and cluster ui.
  public void apply() {
    update();
    // cluster_table_update(this, gg);
  }

  public void reset() {
    set_included_all(true);
    apply();
  }

  // Select n random observations
  // Algorithm taken from Knuth, Seminumerical Algorithms; Vol 2
  public bool select_random (int n) {
    bool[] included = new bool[n_rows];

    if (n < 0 || n > n_rows) 
      return false;

    set_included_all(false);
  
    int t, m;
    for (t = 0, m = 0; t < n_rows && m < n; t++) {
      double r = Random.double();
      if (((n_rows - t) * r) < (n - m) && !included[t]) {
        set_included(t, true);
        included[t] = true;
        m++;
      }
    }

    return true;
  }

  // Select a contiguous block of data
  public bool select_block (int start, int size) { 
    int i, k;

    if (start < 0 || start > n_rows || size < 0) {
      GGobi.message("The limits aren't correctly specified.", false);
      return(false);
    }

    set_included_all(false);
    for (i = start, k = 1; i < n_rows && k <= size; i++, k++) {
      set_included(i, true);
    }

    return(true);
  }

  // Select every @step points, starting from @start.
  public bool select_every_n(int start, int step) {

    if (start < 0 || start > n_rows - 1 || step < 0 || step > n_rows) {
      GGobi.message ("Interval not correctly specified.", false);
      return(false);
    }
  
    set_included_all(false);
    for(uint i = start; i < n_rows; i += step)
      set_included(i, true);
  
    return true;
  }

  /**
   * rowlab:
   * @substr: substring matched against rownames
   * @method: matching method
   * @ignore_case: %TRUE for case insensitive matching
   *
   * Subsets records by their row id, according to @method.
   *
   * Returns: %TRUE if subset changed
   */
  public bool select_matching_labels(string substr, MatchMethod method, bool ignore_case) {
    bool value;
    string search = substr; // needed for vala
  
    if (substr == null || substr.len() == 0) {
      set_included_all(true);
      return true;
    }

    set_included_all(false);
    if (ignore_case) search = search.down();

    for (uint i = 0; i < n_rows; i++) {
      string label = get_row_id(i);
    
      if (label == null) continue;
      if (ignore_case) label = label.down();
    
      switch(method) {
        case MatchMethod.INCLUDES:
          value = label.str(search) != null;
          break;
        case MatchMethod.EXCLUDES:
          value = label.str(search) == null;
          break;
        case MatchMethod.IDENTICAL:
          value = (label == search);
          break;
        case MatchMethod.ENDS_WITH:
          value = label.has_suffix(search);
          break;
        case MatchMethod.STARTS_WITH:
          value = label.has_prefix(search);        
          break;
      }   
      set_included(i, value);
    }
    return true;
  }
} 
