import gnu.gtk.*;
import gnu.gdk.GdkEvent;

// The callback test, now coded with Sopwith mechanism
//
// Ol.G -

public class GGobiCallbackTest
{
    public boolean delete_event(GdkEvent dummy) {
        Gtk.mainQuit();
        return true;
    };   
    
    public void clickcall(Object data) {
        System.err.println(data.toString());
    };
    
    
    public GGobiCallbackTest()
    {
	String args[] = {"ggobi"};
        Gtk.init(args.length,args);
        GtkWindow g = new GtkWindow(GtkWindowType.TOPLEVEL);
        GtkButton b = new GtkButton("Feeeeeeed Me !");
        
        g.signalConnect("delete_event", this);
	g.signalConnect("destroy", "mainQuit", Gtk.class);
        b.signalConnect("clicked", "clickcall", this, new String("TEST"));
        GtkTooltips t = new GtkTooltips();
        t.setTip(b, "Toto (french for foo)", "Hidden Data");

        b.show();
        g.setUsize(300,50);
        g.add(b);
        g.show();
	//        Gtk.main();
    }     
}




