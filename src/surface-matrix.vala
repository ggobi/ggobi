/* A surface that consists of a grid of subsurfaces */
public interface GGobi.Surface.Matrix : Container {
  public abstract uint n_rows { get; construct set; }
  public abstract uint n_cols { get; construct set; }
  public abstract Surface get_element(uint row, uint col);
}