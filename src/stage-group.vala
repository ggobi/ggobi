/* "Groups" the rows according to a list of factors. The group assignments
   are communicated as columns appended on the data matrix. Each column
   crosses another factor with the factors of the previous column. For example,
   for factors A, B, and C, the grouping columns would be A, AxB, and AxBxC.
   The counts for each column are easily obtained from the GGobiVariable.
*/
class GGobi.StageGroup : Stage {
  
  private List<string> factors;
  private double groups;

  public List<string> get_factors() {
    factors.copy;
  }

  public void insert_factor(string factor, int pos) {
    uint index = get_col_index_for_name(factor);
    return_if_fail(index >= 0);
  
    factors = factors.insert(factor, pos);

    groups_dirty();
    cols_added(1);
    flush_changes_here();
    get_variable(n_cols - 1).set_vartype(GGOBI_VARIABLE_CATEGORICAL);
  }

  public void remove_factor(string factor) {
    String element = factors.find_custom(factor, strcmp);

    if (element) {
      factor = factors.delete_link(element);
      groups_dirty();
      col_removed(n_cols - 1);
      flush_changes_here();
    }
  }

  private void groups_dirty() {
    groups = NULL;
  }

  override void set_missing(uint i, uint j) {
    // FIXME: what happens when a missing is set on a group column?
  }

  override bool is_missing(uint i, uint j) {
    /* Negative values indicate missings */
    if (j >= parent->n_cols) return get_raw_value(i, j) < 0;
    return is_missing(i, j);  
  }

  override void set_raw_value(uint i, uint j, double value) {
    if (j >= parent->n_cols)
      set_levels_for_group(i, j - parent->n_cols, value);
    else parent.set_raw_value(i, j, value)
  }

  override double get_raw_value(uint i, uint j) {
    if (j >= parent->n_cols) {
      find_groups();
      return groups[n_rows * (j - parent->n_cols) + i];
    }
    return get_raw_value(i, j);
  }

  private void set_levels_for_group(uint i, uint j, double value) {
    List factors;
    find_groups();

    foreach (string factor in factors) {
      Variable var = get_variable(get_col_index_for_name(factor));

      uint nlevels = var.get_n_levels();
      uint values[] = var.get_level_values(var);
      /* the % operator doesn't work on doubles */
      double q = groups[j * n_rows + i] / nlevels;
      set_raw_value(i, col, values[(uint)(q - (uint)q) * nlevels]);
    }
  }

  private void find_groups() {
    if (groups != NULL) return;
    bool first = true;
  
    groups = List<string>.new(factors.length * n_rows);
    foreach (string factor in factors) {
      Variable var = get_variable(get_col_index_for_name(factor));
      uint nlevels = var.get_n_levels();

      uint i;
      for (i = 0; i < n_rows; i++)
        p->groups[j * n_rows + i] = 
          (j > 0 ? groups[(j-1) * n_rows + i] : 0) * nlevels +
          var.get_level_index(get_raw_value(i, col));

      first = false;
    }
  }

}

