/* A surface that is a stack of subsurfaces */
public interface GGobi.Surface.Stack : Container {
  /* Depth order */
  public abstract void raise_child(Surface child, Surface? sibling);
  public abstract void lower_child(Surface child, Surface? sibling);

  /* Creates a child and pushes it on top of the stack */
  public abstract Buffer create_buffer();
  public abstract Stack create_stack();
  public abstract Matrix create_matrix(int n_rows, int n_cols);
}