package ggobi;

/**
   A basic plugin that implements the methods of the interface.
   One can extend this to implement the methods with trivial
   implementations. However, doing so will incur the small
   but non-zero expense of  a call to a Java method.
   If one provides a class that is not a `plugin' interface
   then the methods will not be found. This is dubious and
   may not be supported in the future. We can cache method
   identifiers if we know we are dealing with `plugin' instances
   and so speed things up.
 */
public class TestPlugin 
         implements plugin
{
    ggobi gg;
    public TestPlugin() {
	System.err.println("In TestPlugin constructor");
    }

    public TestPlugin(ggobi inst) {
	this();
	System.err.println("In TestPlugin constructor with ggobi instance");
        gg = inst;
    }

    public boolean onCreate() {
	System.err.println("In TestPlugin onCreate()");
	return(true);
    }

    public boolean onClose() {
	System.err.println("In TestPlugin onClose()");
	return(true);
    }

    public boolean onUpdateDisplay() {
	System.err.println("In TestPlugin onUpdateDisplay()");
	return(true);
    }

    static public boolean onUnload() {
 	System.err.println("In TestPlugin onUnload()");
        return(true);
    }

    static public boolean onLoad() {
 	System.err.println("In TestPlugin onLoad()");
        return(true);
    }
}
