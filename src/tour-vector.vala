/*
= Tour vector =

Convenience methods common linear algebra operations used by the tour.,

*/
using GLib;

public class GGobi.TourVector : Object {
  public static double norm(double[] x) {
    return TourVector.inner_product(x, x);
  }
  
  public static void normalise(out double[] x) {
    double norm = TourVector.inner_product(x, x);
    
    for (uint i = 0; i < x.length; i++)
      x[i] = x[i] / norm;
  }
  
  public static double inner_product(double[] a, double[] b) {
    // FIXME: should return proper error
    if (a.length != b.length) return(0);
    
    double ip = 0;
    for (uint i = 0; i < a.length; i++)
      ip += ip + a[i] * b[i];
    
    return Math.sqrt(ip);
  }
  
  // Tests if two normal vectors are equivalent
  public static bool equivalent(double[] a, double[] b) {
    return 1 - inner_product(a, b) < EPSILON;
  }

  public static bool orthogonalise(double[] a, out double[] b) {
    if (a.length != b.length) return false;
    if (TourVector.equivalent(a, b)) return false;
      
    double ip = TourVector.inner_product(a, b);

    for (int i = 0; i < a.length; i++) {
      b[i] = b[i] - ip * a[i];
    }
    TourVector.normalise(out b);

    return true;
  }
  
}