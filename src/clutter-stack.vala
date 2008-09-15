/* Implement GGobi.Surface.Stack with a Clutter.Group */

public class GGobi.Surface.ClutterStack : ClutterSurface, Container, Stack
{
  private Clutter.Group group = new Clutter.Group();
  
  /* Managing child collection */
  public List<Surface> get_children() {
    return group.get_children().copy();
  }
  
  /* Depth order */
  public void raise_child(Surface child, Surface? sibling) {
    group.raise_child(child as ClutterSurface, sibling as ClutterSurface);
  }
  public void lower_child(Surface child, Surface? sibling) {
    group.lower_child(child as ClutterSurface, sibling as ClutterSurface);
  }

  /* Delegating to Clutter.Group */
  // FIXME: would be nice to share this with ClutterMatrix
  // But we probably don't want to expose the Clutter.Group
  public override void paint() {
    group.paint();
  }
  public override void pick(Clutter.Color color) {
    group.pick(color);
  }
  public override void realize() {
    group.realize();
  }
  public override void unrealize() {
    group.unrealize();
  }
  public override void hide_all() {
    group.hide_all();
  }
  public override void show_all() {
    group.show_all();
  }
  /*
  public override void get_preferred_width(Clutter.Unit for_height,
                                           out Clutter.Unit min_width_p,
                                           out Clutter.Unit natural_width_p)
  {
    group.get_preferred_width(for_height, out min_width_p, out natural_width_p);
  }
  public override void get_preferred_height(Clutter.Unit for_width,
                                            out Clutter.Unit min_height_p,
                                            out Clutter.Unit natural_height_p)
  {
    group.get_preferred_height(for_width, out min_height_p,
                               out natural_height_p);
  }
  */
  
  /* This is where we will "stack" the children, eventually */
  public override void allocate(Clutter.ActorBox box,
                                bool absolute_origin_changed) {
    group.allocate(box, absolute_origin_changed);
  }

  /* Child factory methods */

  public Buffer create_buffer() {
    ClutterBuffer buffer = new ClutterBuffer();
    group.add(buffer);
    return buffer;
  }
  public Stack create_stack() {
    ClutterStack stack = new ClutterStack();
    group.add(stack);
    return stack;
  }
  public Matrix create_matrix(int n_rows, int n_cols) {
    ClutterMatrix matrix = new ClutterMatrix(n_rows, n_cols);
    group.add(matrix);
    return matrix;
  }
}