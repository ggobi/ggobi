import gnu.gtk.*;

public class GtkColorDialog {

    public GtkColorDialog() {
        String[] args = {"ggobi"};
        Gtk.init(args.length, args);
	GtkColorSelectionDialog dlg = new GtkColorSelectionDialog("color selection dialog");
        dlg.show();
	//        Gtk.main();
    }
}
