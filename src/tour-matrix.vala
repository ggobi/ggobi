/*
= Tour matrix =

This matrix wrapper implements many of the linear algebra operations needed
for the tour.

*/
using GLib;

public class GGobi.TourMatrix : PipelineMatrix {
  public TourMatrix(uint n_cols, uint n_rows) {
    this.n_cols = n_cols;
    this.n_rows = n_rows;
  }
  construct {
    matrix = new Matrix(n_cols, n_rows);
  }
  
  // Returns column of matrix
  public weak double[] col(uint j) {
    weak double[] tmp = ((double[]) matrix.vals[j]);
    tmp.length = (int) n_rows; 
    return tmp;
  }
  
  // Is this matrix orthonormal?
  public bool is_orthonormal() {
    for(uint j = 0; j < n_cols; j++) {
      if (Math.fabs(1 - col_norm(j)) > EPSILON) 
        return false;
    }
    
    for (uint j = 0; j < n_cols; j++) {
      for (uint k = j + 1; k < n_cols; j++) {
        if (col_inner_product(j, k) > EPSILON)
          return false;
      }
    }
    return true;
    
  }
  public bool orthogonalize_by(TourMatrix other) {
    weak double[] left;
    weak double[] right;

    for (uint j = 0; j < n_cols; j++) {
      left = other.col(j);
      right = col(j);
      
      if (!TourVector.orthogonalize(left, right))
        return false;
    }
    return true;
  }
  public bool orthogonalize() {
    if (n_cols == 1) return(true);
    
    weak double[] left;
    weak double[] right;
    
    for (uint j = 0; j < n_cols; j++) {
      for (uint k = j + 1; k < n_cols; j++) {
        left = col(j);
        right = col(k);
        
        if (!TourVector.orthogonalize(left, right)) return false;
      }
    }    
    return true;
  }

  // Normalize entire matrix
  public void normalize() {
    for (uint j = 0; j < n_cols; j++)
      normalize_col(j);
  }

  // Normalize given column
  public void normalize_col(uint j) {
    weak double[] tmp = col(j);
    TourVector.normalize(tmp);
  }
  
  // Calculate norm (length) for specified column
  public double col_norm(uint j) {
    return col_inner_product(j, j);
  }

  // Compute inner product between two columns
  public double col_inner_product(uint a, uint b) {
    return TourVector.inner_product(col(a), col(b));
  }

  // Return a transposed copy of the matrix
  public TourMatrix transpose() {
    TourMatrix mat = new TourMatrix(n_cols, n_rows);
    for(uint i = 0; i < n_rows; i++) {
      for(uint j = 0; j < n_rows; j++) {
        mat.set(j, i, get(i, j));
      }
    }
    return mat;
  }  

  public TourMatrix copy() {
    TourMatrix mat = new TourMatrix(n_rows, n_cols);
    for(uint i = 0; i < n_rows; i++) {
      for(uint j = 0; j < n_rows; j++) {
        mat.set(i, j, get(i, j));
      }
    }
    return mat;
  }
  
  // Wraps up svd method from svd.c so it doesn't modify anything it shouldn't
  // and returns everything in a nice little struct.
  public Svd svd() {
    TourMatrix U = copy();
    TourMatrix Vt = new TourMatrix(n_rows, n_cols);
    double[] d = new double[0];
    d.resize((int) n_rows);
    
    LinearAlgebra.svd(U.matrix.vals, (int) n_rows, (int) n_cols, d, Vt.matrix.vals);
    
    Svd svd = new Svd(U, Vt.transpose());
    svd.set_d(d);
    return svd;
  }
  
  public bool equivalent(TourMatrix other) {
    for (uint j = 0; j < n_cols; j++) {
      if (!TourVector.equivalent(col(j), other.col(j))) return false;
    }
    return true;
  }
  
  public static TourMatrix? multiply_uv(TourMatrix u, TourMatrix v) {
    TourMatrix mat = new TourMatrix(v.n_cols, u.n_rows);

    // FIXME: throw exception?
    if (u.n_cols != v.n_rows) return null;

    for (uint j = 0; j < u.n_rows; j++) {
      for (uint k = 0; k < v.n_cols; k++) {
        double value = 0;
        for (uint i = 0; i< u.n_cols; i++) {
          value += u.get(i, j) * v.get(k, i);
        }
        mat.set(k, j, value);
      }
    }

    return mat;
  } 
  public static TourMatrix? multiply_utv(TourMatrix u, TourMatrix v) {
    TourMatrix mat = new TourMatrix(u.n_rows, u.n_cols);

    // FIXME: throw exception?
    if (u.n_rows != v.n_rows) return null;

    for (uint j = 0; j < u.n_cols; j++) {
      for (uint k = 0; k < v.n_cols; k++) {
        double value = 0;
        for (uint i = 0; i< u.n_rows ; i++) {
          value += u.get(j, i) * v.get(k, i);
        }
        mat.set(k, j, value);
      }
    }
    
    return mat;
  } 
  // Not currently used:
  // public static TourMatrix multiply_uvt(TourMatrix u, TourMatrix v) {
  //   TourMatrix mat = new TourMatrix(u.n_rows, u.n_cols);
  //   
  //   if (u.n_cols != v.n_cols) return null;
  // 
  //   for (uint j = 0; j < u.n_rows; j++) {
  //     for (uint k = 0; k < v.n_rows; k++) {
  //       double value = 0;
  //       for (uint i = 0; i< u.n_cols ; i++) {
  //         value += u.get(i, j) * v.get(i, k);
  //       }
  //       mat.set(k, j, value);
  //     }
  //   }
  //   
  //   return mat;
  // } 
  
  public TourMatrix project(Stage stage) {
    TourMatrix mat = new TourMatrix(stage.n_rows, n_cols);
    
    for (uint i = 0; i < stage.n_rows; i++) {
      for (uint j = 0; j < n_rows; j++) {
        double value = 0;
        for (uint k = 0; k < stage.n_cols; k++) {
          value += stage.get_raw_value(i, k) * get(j, k);
        }
        mat.set(i, j, value);
      }
    }    
    return mat;
  }
  
  
}
