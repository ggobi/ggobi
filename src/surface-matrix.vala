/* A surface that consists of a grid of subsurfaces */
public interface GGobi.SurfaceMatrix : SurfaceContainer {
  public abstract int n_rows { get; construct set; }
  public abstract int n_cols { get; construct set; }
  public abstract Surface get_element(int row, int col);
}