public class GGobi.Test.Surface {

  private static void paint_buffer(GGobi.Surface.ClutterBuffer buffer,
                                   GGobi.Surface.Drawable drawable)
  {
    GGobi.Surface.Color black = { 0, 0, 0, 1 };
    drawable.set_stroke(black);
    for (int i = 5; i <= 100; i+=5)
      drawable.draw_circle(100, 100, i);
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