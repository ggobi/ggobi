/* 
Convenience struct for wrapping up results of svd. 
*/

using GLib;

public class GGobi.Svd : Object {
  public TourMatrix U {construct; get;}
  public TourMatrix V {construct; get;}
  public double[] d;
  
  public Svd(TourMatrix U, TourMatrix V) {
    this.U = U;
    this.V = V;
  }
  
  public void set_d(double[] value) {
    d.resize(value.length);
    for (uint i = 0; i < value.length; i++) {
      d[i] = value[i];
    }
  }
}