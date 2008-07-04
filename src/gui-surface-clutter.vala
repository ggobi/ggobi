public class GGobi.GuiSurfaceClutter : GtkClutter.Embed, GuiSurface {
  private SurfaceStack _surface;
  public SurfaceStack surface { get; }

  construct {
    _surface = new SurfaceStackClutter();
    get_stage().add_actor(_surface);
  }
}