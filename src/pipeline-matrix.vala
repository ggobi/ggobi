/* 
= PipelineMatrix =

This class wraps the matrix class with methods that are useful for the 
pipeline, particularly correctly resizing and initialising the matrix in 
response to pipeline messages.

It also has the advantage that we can extend behaviour by writing vala code,
although this will change when we eventually convert the Matrix class into
pure vala code.

*/

using GLib;

public class GGobi.PipelineMatrix : GLib.Object {
  public uint n_cols {get {return matrix.n_cols;} construct;}
  public uint n_rows {get {return matrix.n_rows;} construct;}
  public Matrix matrix;
  
  construct {
    matrix = new Matrix(0, 0);
  }
  
  public double get(uint i, uint j) {
    return ((double[]) matrix.vals[i])[j] ;
  }
  
  public void set(uint i, uint j, double value) {
    ((double[]) matrix.vals[i])[j] = value;
  }
  
  public void add_rows(uint nr) {
    matrix.add_rows((int) nr);
  }
  
  public void add_cols(uint nc) {
    matrix.add_cols((int) nc);
  }

  public void process_message(PipelineMessage msg, Stage stage) {
    SList removed_rows = msg.get_removed_rows();
    matrix.remove_rows(removed_rows);
    matrix.add_rows((int) stage.n_rows);

    SList removed_cols = msg.get_removed_cols();
    matrix.remove_cols(removed_cols);
    matrix.add_cols((int) stage.n_cols);
        
    SList changed_cols = msg.get_changed_cols();
    uint n_added_cols = msg.get_n_added_cols();
    uint n_added_rows = msg.get_n_added_rows();
    uint n_refresh = n_added_cols;

    if (removed_rows != null || n_added_rows > 0) {
      // if rows changed, update all columns
      n_refresh = n_cols; 
    } else {
      foreach(uint j in changed_cols) {
        // GLib.debug("Updating column %i", j);
        stage.refresh_col_(j);
      }
    }
    for (uint j = n_cols - n_refresh; j < n_cols; j++) {
      stage.refresh_col_(j);
      // GLib.debug("Updating column %i", j);     
    }
    
  }
}