public class GGobi.Test.Surface {

  private static void paint_buffer(GGobi.Surface.ClutterBuffer buffer,
                                   GGobi.Surface.Drawable drawable)
  {
    int r = 5;
    GGobi.Surface.Color color = { 0, 0, 0, 1 };
    int cx = 100, cy = 100;
    drawable.set_fill(color);
    TimeVal before = TimeVal();
    for (int j = 0; j < 30000; j++) {
      drawable.draw_circle(cx, cy, r);
    }
    TimeVal after = TimeVal();
    debug("opengl: %f", after.tv_sec + (float)after.tv_usec/1000000 -
          (before.tv_sec + (float)before.tv_usec/1000000));

    Gdk.Pixmap pixmap = new Gdk.Pixmap(null, WINWIDTH, WINHEIGHT, 24);
    Gdk.GC gc = new Gdk.GC(pixmap);
    before = TimeVal();
    for (int j = 0; j < 30000; j++) {
      Gdk.draw_arc (pixmap, gc, false, cx - r, cy - r, 2 * r, 2 * r, 0, 23040);
      Gdk.draw_arc (pixmap, gc, true, cx - r, cy - r, 2 * r, 2 * r, 0, 23040);
    }
    after = TimeVal();
    debug("gdk: %f", after.tv_sec + (float)after.tv_usec/1000000 -
          (before.tv_sec + (float)before.tv_usec/1000000));

    Gdk.Pixbuf target = new Gdk.Pixbuf(Gdk.Colorspace.RGB, true, 8, WINWIDTH,
                                       WINHEIGHT);
    Gdk.Pixbuf glyph = new Gdk.Pixbuf(Gdk.Colorspace.RGB, true, 8, 2*r, 2*r);
    Gdk.pixbuf_get_from_drawable(glyph, pixmap, null, cx - r, cy - r, 0, 0, 2*r,
                                 2*r);
    before = TimeVal();
    for (int j = 0; j < 30000; j++) {
      glyph.composite(target, 0, 0, 2*r, 2*r, 0, 0, 1, 1,
                      Gdk.InterpType.NEAREST, 255);
    }
    after = TimeVal();
    debug("pixbuf: %f", after.tv_sec + (float)after.tv_usec/1000000 -
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