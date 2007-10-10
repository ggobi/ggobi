using GLib;

/**
 * SECTION:GGobiData
 * @short_description: Basic data storage and manipulation class.
 * @stability: Unstable
 *
 * The GGobiData class provides all methods for manipulating 
 * data within GGobi.  It is the starting point for the pipeline.
 * 
 * Eventually this will become an abstract class and subclass will
 * provide the actual implementation.  This will make it easy to treat
 * an R data frame, or a SQL query as the data source for GGobi.
 */
public class GGobi.Data : Stage {

  public string nickname {get; set;}
  public InputSource source {get; construct;}

  // The actual data
  private Matrix raw;
  // Per cell missing values
  private Matrix missing; 

  /* Maps row labels to row indices */
  private HashTable<string,int> id_to_row;

  /* The row ids */
  private string[] row_ids;

  construct {
    name = "unknown"; 
    raw = new Matrix(0, 0);
    missing = new Matrix(0, 0);
    id_to_row = new HashTable<string, int>(str_hash, direct_equal);
    add_cols(n_cols);
    add_rows(n_rows);
  }

  public Data(construct uint n_rows, construct uint n_cols) {
  }

  // FIXME: this stuff is _G_Gobi-specific, so it has to be added to every
  // GGobiData that participates in _G_Gobi, regardless of source.
  // Thus, this should probably be done when the GGobiData is added to the 
  // GGobiApp (GGobiSession) object.
  public void add_attributes() {
    double def_color = (double) 0; //(gg ? gg.color_id : 0);
    double def_type  = (double) GlyphType.FC; //(gg ? gg.glyph_id.type : FC);
    double def_size =  (double) 1; //(gg ? gg.glyph_id.size : 1);
  
    // FIXME: we set the default value here, even though the GGobiSession
    // fields change to match the current brush colour
    add_attribute("_color", def_color);
    add_attribute("_color_now", def_color);
    add_attribute("_color_prev", def_color);

    add_attribute("_size", def_size);
    add_attribute("_size_now", def_size);
    add_attribute("_size_prev", def_size);

    add_attribute("_type", def_type);
    add_attribute("_type_now", def_type);
    add_attribute("_type_prev", def_type);

    add_attribute("_hidden", 0);
    add_attribute("_hidden_now", 0);
    add_attribute("_hidden_prev", 0);

    add_attribute("_cluster", 0);
    add_attribute("_sampled", (double) true);
    add_attribute("_excluded", (double) true);

  } 

  // Adds a new attribute column with specified name and default value.
  public void add_attribute(string name, double value) {
    if (get_col_index_for_name(name) != -1)
      return;

    // FIXME: inefficient..
    uint j = add_cols(1);
    n_attributes += 1;

    set_col_name(j, name);
    set_col_default_value(j, value);	
  
    for(uint i = 0; i < n_rows; i++) {
      set_raw_value(i, j, value);
    }

    Variable vt = get_variable(j);
    vt.is_attribute = true;
  }

  public void set_col_default_value(uint j, double value) {
    Variable vt = get_variable(j);
    vt.default_value = value;
  }

  public double get_col_default_value(uint j) {
    Variable vt = get_variable(j);
    return vt.default_value;
  }

  // Set missing value.  For categorical variables, this creates a new
  // value, MISSING.  This makes GGobi act a little more like MANET.
  override void set_missing(uint i, uint j) {
    Variable vt = get_variable(j);
    if (vt.vartype == VariableType.CATEGORICAL) {
      set_categorical_value(i, j, "MISSING");
      return;
    }
    
    ((double[]) raw.vals[i])[j] = 0;
    ((double[]) missing.vals[i])[j] = 1;
  }

  override bool is_missing(uint i, uint j) {
    return ((double[]) missing.vals[i])[j] == 1;  
  }

  override void set_raw_value(uint i, uint j, double value) {
    ((double[]) raw.vals[i])[j] = value;
    ((double[]) missing.vals[i])[j] = 0;
  }

  override double get_raw_value(uint i, uint j) {
    return ((double[]) raw.vals[i])[j];
  }

  public void set_col_type(uint j, VariableType value) {
    Variable vt = get_variable(j);
    vt.vartype = value;
  }

  override uint get_n_edges() {
    return 0;
  }

  override EdgeData get_edge_data () {
    return null;
  }

  override void set_row_id(uint i, string value) {
    string val = value; // needed for vala
    if (val == null)
      val = (i + 1).to_string("%d");
    row_ids[i] = val;
    id_to_row.replace(row_ids[i], i);
  }

  override string get_row_id(uint i) {
    return row_ids[i];
  }

  override int get_row_for_id(string id) { 
    return id_to_row.lookup(id);
  }

  /**
   * Add n rows to the dataset
   * Adds extra rows, allocating space as needed.
   *
   * For convenience, this registers the row change and flushes the changes.
   **/
  // FIXME: Most of this should happen in the outgoing message handler
  public uint add_rows(uint n) {
    uint i, nprev = n_rows;
    uint n_rows = nprev + n;

    row_ids.resize((int) n_rows);
    raw.add_rows((int) n_rows);
    missing.add_rows((int) n_rows);

    // if (get_n_edges())
    //   vectorb_realloc (&self.edge.xed_by_brush, self.edge.n);

    for (i = nprev; i < n_rows; i++) {    
      set_row_id(i, null);
    }
  
    reset_rows(nprev, n_rows);
    rows_added(n);
    flush_changes_here();
  
    return nprev;
  }

  /**
   * Resets values to their defaults for the specified 
   * rows.  This is mainly used to set default values for 
   * the attributes at the moment.
   **/ 
  public void reset_rows(uint start, uint end) {
    for (uint j = 0; j < n_cols; j++) {
      double default_val = get_col_default_value(j);
      for (uint i = start; i < end; i++)
        set_raw_value(i, j, default_val);
    }
  }

  /**
   * Add n empty columns to the dataset.
   * For convenience, this registers the row change and flushes the changes.
   **/ 
   // FIXME: This function and its siblings should probably be moved to GGobiStage,
   // and be written looking to _update_cols(). The function bodies should then
   // be moved to the outgoing message handler of GGobiData.
  public uint add_cols(uint n) {
    uint nprev = n_cols;
    uint n_cols = nprev + n;
  
    raw.add_cols((int) n_cols);
    missing.add_cols((int) n_cols);

    cols_added(n);
    flush_changes_here();
  
    return nprev;
  }

  /**
   * Deletes the specified columns from the dataset. Registers and flushes
   * the change.
   */
  public uint delete_cols(SList<uint> cols) {
    uint old_n_cols = n_cols;
    uint n_cols = cols.length();
  
    // g_return_val_if_fail(n_cols <= old_n_cols, old_n_cols);

    raw.remove_cols(cols);
    missing.remove_cols(cols);
    
    cols_removed(cols);  
    flush_changes_here();
  
    return(n_cols);
  }
}