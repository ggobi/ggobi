package ggobi;

public class datad 
         extends CStruct
{

    /*  What about having a GGobi instance (ggobi.ggobi) field also.  */

    static {
	try {
	    System.loadLibrary("JavaGGobi");
	} catch(Exception e) {
	    System.err.println("Cannot load the DLL JavaGGobi. Check your environment variable LD_LIBRARY_PATH (Unix) or PATH (Windows)");
	    e.printStackTrace();
	}
    } 


    public datad() {
    }

    public datad(double val) {
	super(val);
    }

    native public String getName();
    native public String getNumRecords();
}
