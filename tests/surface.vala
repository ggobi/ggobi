public class GGobi.Test.Surface {
  public static void main(string[] args) {
    /* create a clutter stage embedded in a GTK+ window */

    if (GtkClutter.init(ref args)!=Clutter.InitError.SUCCESS)
      error("Unable to initialize GtkClutter");

    Gtk.Window window = new Gtk.Window(Gtk.WindowType.TOPLEVEL);
    window.destroy += Gtk.main_quit;

    GtkClutter.Embed clutter = new GtkClutter.Embed();
    
    window.add(clutter);
    
    Clutter.Stage stage = (Clutter.Stage)clutter.get_stage();
    if (!(stage is Clutter.Actor))
      debug("not an actor: %p", stage);
    
    /* create a clutter surface */

    GGobi.Surface.ClutterSurface surface = new GGobi.Surface.ClutterSurface();
    
    stage.add(surface);

    /* bring up the GUI */
    
    window.show_all();
    
    Gtk.main();
  }
}