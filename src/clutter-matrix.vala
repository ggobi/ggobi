public class GGobi.Surface.ClutterMatrix : ClutterSurface, Container, Matrix
{
  private Clutter.Group group = new Clutter.Group();

  public uint n_rows { get; construct set; }
  public uint n_cols { get; construct set; }
  
  public ClutterMatrix(uint n_rows, uint n_cols) {
    this.n_rows = n_rows;
    this.n_cols = n_cols;
  }

  public Surface get_element(uint row, uint col) {
    return (Surface)group.get_nth_child((int)(n_rows*col + row));
  }
  
  /* Managing child collection */
  public List<Surface> get_children() {
    return group.get_children().copy();
  }
  
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

  /* This is where we will put the layout logic */
  public override void allocate(Clutter.ActorBox box,
                                bool absolute_origin_changed) {
    group.allocate(box, absolute_origin_changed);
  }
}