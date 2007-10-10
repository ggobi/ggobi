using GLib;

[CCode (cheader_filename = "ggobi-stage.h")]
public class GGobi.Stage : GLib.Object {
  public uint n_rows { get; set construct; }
  public uint n_cols { get; set construct; }
  public string name { get; set construct; }
  public Session gg;
  public uint n_attributes;
  
  public Stage parent {get; set construct; }
  public signal void changed (PipelineMessage msg);
  public signal void col_parameter_changed (uint j);
  public void col_data_changed(uint j);
  public void cols_added(uint n);
  public void cols_removed(SList cols);
  public void rows_added(uint n);
  public void rows_removed(SList rows);

  public void flush_changes_here();
  public virtual void process_incoming(PipelineMessage msg);
  public virtual void process_outgoing(PipelineMessage msg);
  
  public virtual void set_row_id(uint i, string id);
  public virtual string get_row_id(uint i);
  public virtual int get_row_for_id(string id);
  
  public string get_string_value(uint i, uint j);
  public void set_string_value(uint i, uint j, string val);
  public virtual void set_categorical_value(uint i, uint j, string value);
  
  public virtual double get_raw_value(uint i, uint j);
  public virtual void set_raw_value(uint i, uint j, double value);
  public virtual bool is_missing(uint i, uint j);
  public virtual void set_missing(uint i, uint j);
  
  public weak Variable get_variable(uint j);
  public uint get_col_n_missing(uint j);
  public virtual void set_col_name(uint j, string name);
  public int get_col_index_for_name(string name);
  
  public virtual uint get_n_edges();
  public virtual EdgeData get_edge_data();
  
  public virtual void refresh_col_(uint j);
  public virtual void refresh_col(uint j);
  public virtual void refresh_();
  public virtual void refresh();
  
}

public class EdgeData {
  public int n;
}


[CCode (cheader_filename = "ggobi-pipeline-message.h")]
public class GGobi.PipelineMessage : GLib.Object{
  public GLib.SList get_changed_cols();
  public GLib.SList get_removed_cols();
  public GLib.SList get_removed_rows();
  public uint get_n_added_rows();
  public uint get_n_removed_rows();
  public uint get_n_added_cols();
  public uint get_n_changed_cols();
  public uint get_n_removed_cols();
  
  public void consume_rows();
  
}

public class GGobi.Session {
}

[CCode (cheader_filename = "ggobi-variable.h")]
public class GGobi.Variable: GLib.Object {
  public VariableType vartype {get; set;}
  public bool is_attribute {get; set;}
  private double default_value {get; set;}

  public string name;
  
  public double get_range();
  public double get_min();
  public double get_median();
  public double get_max();
  
  public double get_mean();
  public bool has_missings();
  public uint n_missings();
  

}


namespace GGobi {
  [CCode (cprefix = "", lower_case_cprefix = "", cheader_filename = "ggobi.h")]
  public struct Utils {
    public static double random_normal();
  }

  [CCode (cprefix = "d", lower_case_cprefix = "", cheader_filename = "svd.h")]
  public struct LinearAlgebra {
    [NoArrayLength]
    public static void svd (pointer[] a, int m, int n, out double[] w, out pointer[] v);
  }

  [CCode (cheader_filename = "ggobi-variable.h", cprefix = "GGOBI_VARIABLE_" )]  
  public enum VariableType {
    REAL, CATEGORICAL, INTEGER, COUNTER, UNIFORM, ALL_VARTYPES
  }
  
  [CCode (cname = "RedrawStyle", cprefix = "", lower_case_cprefix = "", cheader_filename = "pipeline.h")]
  public enum RedrawStyle {
    NONE, EXPOSE, QUICK, BINNED, FULL, FULL_1PIXMAP
  }
  
  [CCode (cname = "quick_message", cprefix = "", lower_case_cprefix = "", cheader_filename = "utils_ui.h")]
  void message (string message, bool modal);
  
  [CCode (cprefix = "", lower_case_cprefix = "", cheader_filename = "types.h")]
  public enum GlyphType {DOT_GLYPH, PLUS, X, OC, OR, FC, FR, UNKNOWN_GLYPH}
  
  
  
  [CCode (cheader_filename = "pipeline.h", cname="tform_to_world_by_var")]
  public void tform_to_world_by_var (Stage stage, uint j);
  
  [CCode (cheader_filename = "display.h", cname="displays_tailpipe")]
  void       displays_tailpipe (RedrawStyle style, Session stage);
  
}

[CCode (cprefix = "arrayd_", cheader_filename = "array.h", cname="array_d")]
public class GGobi.Matrix {
  public pointer[] vals;
  [CCode (cname = "nrows")]
  public uint n_rows {get; construct;}
  [CCode (cname = "ncols")]
  public uint n_cols {get; construct;}
  
  public Matrix(construct uint n_rows, construct uint n_cols);
  
  public void alloc (int nr, int nc);
  public void free ();

  public void add_cols (int nc);
  public void add_rows (int nr );
  public void copy (GGobi.Matrix arrp_to);

  [CCode (cname = "arrayd_delete_cols")]
  public void remove_cols (SList cols);
  [CCode (cname = "arrayd_delete_rows")]
  public void remove_rows (SList cols);
  public void init_null ();
  public void zero (); 
}

[CCode (cprefix = "vectord_", cheader_filename = "vector.h", cname="vector_d")]
public class GGobi.Vector {
  public double[] els;
  public uint nels;
}