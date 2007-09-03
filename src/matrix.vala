using GLib;

public class GGobi.Matrix : Object {
  public double[,] vals;
  public uint nrows;
  public uint ncols;
  
  public void alloc (int nr, int nc){}
  public void free (int nr, int nc){}

  public void add_cols (int nc){}
  public void add_rows (int nr ){}
  public void copy (GGobi.Matrix arrp_to){}
  
  public void remove_cols (SList cols){}
  public void remove_rows (SList cols){}
  public void init_null (){}
  public void zero (){}
  
  
}
