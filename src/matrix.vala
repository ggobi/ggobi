using GLib;

public class GGobi.Matrix2 : Object {
  /* Multi-dimensional arrays in vala are stored as a single vector.
  vals[i, j] = i * ncols + j); */ 
  public double[,] vals;
  
  public uint n_rows {get; construct;}
  public uint n_cols {get; construct;}
  
  public Matrix2.with_size (uint n_rows, uint n_cols) {
    this.n_rows = n_rows;
    this.n_cols = n_cols;
  }  
  
  construct {    
    vals = new double[n_rows, n_cols];
  }

  /* Add n new cols */
  public void add_cols (uint n) {
    vals.resize((int) n_rows * (int) (n_cols + n));
    
    _n_cols = n_cols + n;
  }

  /* Add n new rows */
  public void add_rows (uint n) {
    vals.resize((int) (n_rows + n) * (int) n_cols);    

    _n_rows = n_rows + n;
  }

  /* Create a copy of this matrix */
  public GGobi.Matrix2 copy (){
    Matrix2 copy = new Matrix2.with_size(n_rows, n_cols);
    for(uint i = 0; i < n_rows; i++) {
      for(uint j = 0; j < n_cols; j++) {
        copy.vals[i, j] = vals[i, j];
      }
    }
    return copy;
  }
  
  public void remove_cols (SList cols) {
    
  }

  public void remove_rows (SList rows) { 
    
  }

  /* Set all values in array to zero */
  public void zero (){
    for(uint i = 0; i < n_rows; i++) {
      for(uint j = 0; j < n_cols; j++) {
        vals[i, j] = 0;
      }
    }
  }
  
  
}
