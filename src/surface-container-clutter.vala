/* Impements GGobi.SurfaceContainer using Clutter.Group */

public class GGobi.SurfaceContainerClutter : SurfaceClutter, SurfaceContainer {
  private Clutter.Group group = new Clutter.Group();

  public SurfaceContainerClutter(Surface parent) {
    this.parent = parent;
  }
  
  /* Managing child collection */
  public List<Surface> get_children() {
    group.get_children();
  }
  
  public void add_child(Surface surface) {
    group.add_actor(surface as SurfaceClutter);
  }
  
  public void remove_child(Surface surface) {
    group.remove_actor(surface as SurfaceClutter);
  }
  
  /* Depth order */
  public void raise_child(Surface child, Surface? sibling) {
    group.raise_child(child as SurfaceClutter, sibling as SurfaceClutter);
  }
    
  public void lower_child(Surface child, Surface? sibling) {
    group.lower_child(child as SurfaceClutter, sibling as SurfaceClutter);
  }

  /* Delegating to ClutterGroup */
  
  override void paint() {
    group.paint();
  }
  override void pick(Clutter.Color color) {
    group.pick(color);
  }
  override void realize() {
    group.realize();
  }
  override void unrealize() {
    group.unrealize();
  }
  override void hide_all() {
    group.hide_all();
  }
  override void show_all() {
    group.show_all();
  }
  override void get_preferred_width(Clutter.Unit for_height,
                                    ref Clutter.Unit min_width_p,
                                    ref Clutter.Unit natural_width_p)
  {
    group.get_preferred_width(for_height, min_width_p, natural_width_p);
  }
  override void get_preferred_height(Clutter.Unit for_width,
                                     ref Clutter.Unit min_height_p,
                                     ref Clutter.Unit natural_height_p)
  {
    group.get_preferred_height(for_width, min_height_p, natural_height_p);
  }
  override void allocate(Clutter.ActorBox box, bool absolute_origin_changed) {
    group.allocate(box, absolute_origin_changed);
  }

  /* Child factory methods */

  public SurfaceBuffer create_buffer() {
    return new SurfaceBufferClutter(this);
  }
  public SurfaceContainer create_container() {
    return new SurfaceContainerClutter(this);
  }
  public SurfaceMatrix create_matrix(int n_rows, int n_cols) {
    return new SurfaceMatrixClutter(this, n_rows, n_cols);
  }
}