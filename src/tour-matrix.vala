/*
= Tour matrix =

This matrix wrapper implements many of the linear algebra operations needed
for the tour.

*/

class GGobi.PipelineMatrix : Object {
  construct {
    matrix = new Matrix(n_cols, n_rows);
  }
  public void TourMatrix(construct uint n_cols, construct uint n_rows) {}
  
  // Returns column of matrix
  public weak double[] col(uint j) {
    return ((double[]) matrix.vals[j]) 
  }
  
  // Is this matrix orthonormal?
  public bool is_orthonormal() {
    for(uint j = 0; j < n_cols; j++) {
      if (Math.fabs(1 - col_norm(j)) > EPSILON ) 
        return(false);
    }
    
    for (uint j = 0; j < n_cols; j++) {
      for (uint k = j + 1; k < n_cols; j++) {
        if (col_inner_product(j, k)) > EPSILON 
          return(false);
      }
    }
    
  }
  public bool orthogonalise(TourMatrix other) {
    for (uint j = 0; j < n_cols; j++) {
      if (!TourVector.orthongalise(other.col(j), out col(j))) return false;
    }
    return true;
  }
  public bool orthogonalise_self() {
    for (uint j = 0; j < n_cols; j++) {
      for (uint k = j + 1; k < n_cols; j++) {
        if (!TourVector.orthoganalise(col(j), col(k))) return false;
      }
    }    
    return true;
  }

  // Normalise entire matrix
  public void normalise() {
    for (uint j = 0; j < n_cols; j++)
      normalise_col(j);
  }

  // Normalise given column
  public void normalise_col(uint j) {
    TourVector.normalise(col(j));
  }
  
  // Calculate norm (length) for specified column
  public double col_norm(uint j) {
    return col_inner_product(j, j);
  }

  // Compute inner product between two columns
  public double col_inner_product(uint a, uint b) {
    TourVector.inner_product(col(a), col(b))
  }

  // Return a transposed copy of the matrix
  public TourMatrix transpose() {
    TourMatrix mat = new TourMatrix(n_cols, n_rows);
    for(uint i = 0; i < n_rows; i++) {
      for(uint j = 0; j < n_rows; j++) {
        mat.set(j, i, get(i, j))
      }
    }
    return mat;
    
  }  

  public TourMatrix copy() {
    TourMatrix mat = new TourMatrix(n_rows, n_cols);
    for(uint i = 0; i < n_rows; i++) {
      for(uint j = 0; j < n_rows; j++) {
        mat.set(i, j, get(i, j))
      }
    }
    return mat;
  }
  
  public void svd(out double[] d, out Matrix v) {}
  
  public bool equivalent(TourMatrix other) {
    for (uint j = 0; j < n_cols; j++) {
      if (!TourVector.equivalent(col(j), other.col(j))) return false;
    }
    return true;
  }
  
  public static TourMatrix multiply_uv(TourMatrix u, TourMatrix v) {} 
  public static TourMatrix multiply_utv(TourMatrix u, TourMatrix v) {} 
  public static TourMatrix multiply_uvt(TourMatrix u, TourMatrix v) {} 
  
}
