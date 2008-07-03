/* A surface that contains other surfaces, forming a scene graph */

/* Child surfaces represent some subregion of their parent and
   can overlap.  */

public interface GGobi.SurfaceContainer : Surface {
  /* Managing child collection */
  public abstract List<Surface> get_children();
  public abstract void add_child(Surface surface);
  public abstract void remove_child(Surface surface);

  /* Depth order */
  public abstract void raise_child(Surface child, Surface sibling);
  public abstract void lower_child(Surface child, Surface sibling);

  /* Creating children */
  public abstract SurfaceBuffer create_buffer();
  public abstract SurfaceContainer create_container();
  public abstract SurfaceMatrix create_matrix(int n_rows, int n_cols);
}