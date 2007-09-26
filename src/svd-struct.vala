/* 
Convenience struct for wrapping up results of svd. 
*/
public struct GGobi.Svd {
  public TourMatrix U;
  public TourMatrix V;
  public weak double[] d;
}