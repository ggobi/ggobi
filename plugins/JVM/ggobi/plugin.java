package ggobi;

/**
     A basic interface for a GGobi plugin implemented in Java.
     None of the methods currently take any arguments as they can and are expected to 
     store the state themselves.
     The  onLoad and onUnload functionality should be provided as
     static methods in a plugin class.
*/

public interface plugin {
    /**
      This is essentially the constructor and can be deprecated
      for Java or may be changed to accept an object of class
      ggobi.ggobi which is the proxy for the C-level GGobi instance (ggobid).
     */
    public boolean onCreate();

    /**
      Called when the ggobi instance no longer needs this plugin instance.
     */
    public boolean onClose();
    /**
       Potentially called each time the Display menu in this ggobi instance
       is re-created.
     */
    public boolean onUpdateDisplay();
}
