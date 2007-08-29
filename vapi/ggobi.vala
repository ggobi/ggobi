[CCode (cheader_filename = "ggobi-stage.h")]
public class GGobi.Stage {
  public uint n_rows { get; set construct; }
  public uint n_cols { get; set construct; }
  public string name { get; set construct; }
  
  public void set_col_name(uint j, string name);
  public void set_row_id(uint i, string id);
  public void set_string_value(uint i, uint j, string val);
}
[CCode (cheader_filename = "ggobi-data.h")]
public class GGobi.Data : GGobi.Stage {
  public GGobi.InputSource source { get; set construct; }
  
  public void add_attributes();
  
  public Data(uint nrows, uint ncols);
}
