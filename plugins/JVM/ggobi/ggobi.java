package ggobi;

public class ggobi {

    static {
	try {
	    System.loadLibrary("JavaGGobi");
	} catch(Exception e) {
	    System.err.println("Cannot load the DLL JavaGGobi. Check your environment variable LD_LIBRARY_PATH (Unix) or PATH (Windows)");
	    e.printStackTrace();
	}
    } 

    double address;

    public ggobi() {
	System.err.println("Initializing Java-GGobi");
    }

    public ggobi(double address) {
	this.address = address;
    }

  /**
    Dataset-related functions.
   */

    public native String getDescription();

    public native int getNumDatasets();
    public native datad getDataset(int which);
    public native datad getDataset(String name);
}
