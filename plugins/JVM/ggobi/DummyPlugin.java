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
public class DummyPlugin 
         implements plugin
{
    ggobi gg;
    public DummyPlugin() {
    }

    public DummyPlugin(ggobi inst) {
	this();
        gg = inst;
    }

    public boolean onCreate() {
	return(true);
    }

    public boolean onClose() {
	return(true);
    }

    public boolean onUpdateDisplay() {
	return(true);
    }
}
