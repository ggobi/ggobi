public class GGobi.Test.Surface {

  private static void paint_buffer(GGobi.Surface.ClutterBuffer buffer,
                                   GGobi.Surface.Drawable drawable)
  {
    int r = 5;
    GGobi.Surface.Color color = { 1, 0, 0, 1 };
    int cx = 10, cy = 10;
    int x = 300000;

    //drawable.set_fill(color);
    //drawable.draw_circle(cx, cy, r);
    drawable.set_stroke(color);
    //drawable.draw_circle(cx + 3*r, cy, r);
    //drawable.set_fill(null);
    TimeVal before = TimeVal();
    for (int i = 0; i < x; i++)
      drawable.draw_circle(cx + 6*r, cy, r);
    TimeVal after = TimeVal();
    debug("stencil: %f", after.tv_sec + (float)after.tv_usec/1000000 -
          (before.tv_sec + (float)before.tv_usec/1000000));
  }

  private static const int WINWIDTH = 400;
  private static const int WINHEIGHT = 400;
  
  public static void main(string[] args) {
    Clutter.Color stage_color = { 0x61, 0x64, 0x8c, 0xff };
  
    /* create a clutter stage embedded in a GTK+ window */
    
    if (GtkClutter.init(ref args) != Clutter.InitError.SUCCESS)
      error("Unable to initialize GtkClutter");

    Gtk.Window window = new Gtk.Window(Gtk.WindowType.TOPLEVEL);
    window.set_default_size(WINWIDTH, WINHEIGHT);
    window.destroy += Gtk.main_quit;

    GtkClutter.Embed clutter = new GtkClutter.Embed();
    
    window.add(clutter);
    
    Clutter.Stage stage = (Clutter.Stage)clutter.get_stage();

    stage.set_color(stage_color);

    /* create a clutter surface */

    GGobi.Surface.ClutterBuffer buffer = new GGobi.Surface.ClutterBuffer();

    buffer.paint_buffer += paint_buffer;
    
    stage.add(buffer);
    
    /* bring up the GUI */
    
    window.show_all(); /* calls 'show' on the stage, but nothing else */
    buffer.shown = true; /* explicitly show the buffer */
    
    Gtk.main();
  }
}