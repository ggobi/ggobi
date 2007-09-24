using GLib;

[CCode (cheader_filename = "ggobi-stage.h")]
public class GGobi.Stage : GLib.Object {
  public uint n_rows { get; set construct; }
  public uint n_cols { get; set construct; }
  public string name { get; set construct; }
  public Session gg;
  
  public Stage parent {get; set construct; }
  public signal void changed (PipelineMessage msg);
  public signal void col_parameter_changed (uint j);
  
  public virtual void set_col_name(uint j, string name);
  public virtual void set_row_id(uint i, string id);
  public void set_string_value(uint i, uint j, string val);
  public virtual string get_row_id(uint i);
  
  public string get_string_value(uint i, uint j);
  public virtual double get_raw_value(uint i, uint j);
  public virtual void set_raw_value(uint i, uint j, double value);
  public virtual bool is_missing(uint i, uint j);
  
  public uint get_col_n_missing(uint j);
  
  public weak Variable get_variable(uint j);
  
  public void col_data_changed(uint j);
  public void flush_changes_here();
  
  virtual void process_incoming(PipelineMessage msg);
  public virtual void process_outgoing(PipelineMessage msg);
  
  public virtual void refresh_col_(uint j);
  public virtual void refresh_col(uint j);
  public virtual void refresh_();
  public virtual void refresh();
  
}

[CCode (cheader_filename = "ggobi-data.h")]
public class GGobi.Data : GGobi.Stage {
  public GGobi.InputSource source { get; set construct; }
  
  public void add_attributes();
  
  public Data(uint nrows, uint ncols);
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
}

public class GGobi.Session {
}

[CCode (cheader_filename = "ggobi-variable.h")]
public class GGobi.Variable: GLib.Object {
  public VariableType vartype {get; set;}
  public bool is_attribute {get; set;}
  public string name;
  
  public float get_range();
  public float get_min();
  public float get_median();
  public float get_max();
  
  public float get_mean();
  public bool has_missings();

}


namespace GGobi {
  [CCode (cprefix = "", lower_case_cprefix = "", cheader_filename = "ggobi.h")]
  public struct Utils {
    public static double random_normal();
  }

  public const double EPSILON = 0.001; 


  [CCode (cheader_filename = "ggobi-variable.h", cprefix = "GGOBI_VARIABLE_" )]  
  public enum VariableType {
    REAL, CATEGORICAL, INTEGER, COUNTER, UNIFORM, ALL_VARTYPES
  }
  
  [CCode (cname = "RedrawStyle", cprefix = "", lower_case_cprefix = "", cheader_filename = "pipeline.h")]
  public enum RedrawStyle {
    NONE, EXPOSE, QUICK, BINNED, FULL, FULL_1PIXMAP
  }
  
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