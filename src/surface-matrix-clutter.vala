public class GGobi.SurfaceMatrixClutter : SurfaceClutter, Surface,
  SurfaceContainer, SurfaceMatrix
{
  private Clutter.Group group = new Clutter.Group();

  public SurfaceMatrixClutter(Surface parent) {
    this.parent = parent;
  }
  
  /* Managing child collection */
  public List<Surface> get_children() {
    group.get_children();
  }
  
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

  /* This is where we will put the layout logic */
  override void allocate(Clutter.ActorBox box, bool absolute_origin_changed) {
    group.allocate(box, absolute_origin_changed);
  }
}