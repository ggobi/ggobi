package ggobi;

/**
     None of the methods take any arguments as they can and are expected to 
     store the state themselves.
*/

public interface plugin {

    /*
 onLoad and onUnload are static methods in a class.
     */


    /**
      This is essentially the constructor and can be deprecated
      for Java.
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
