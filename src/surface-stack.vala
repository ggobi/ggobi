/* A surface that is a stack of subsurfaces */
public interface GGobi.SurfaceStack : SurfaceContainer {
  /* Depth order */
  public abstract void raise_child(Surface child, Surface sibling);
  public abstract void lower_child(Surface child, Surface sibling);

  /* Creates a child and pushes it on top of the stack */
  public abstract SurfaceBuffer create_buffer();
  public abstract SurfaceStack create_stack();
  public abstract SurfaceMatrix create_matrix(int n_rows, int n_cols);
}