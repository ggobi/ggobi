package ggobi;

public class RandomPlugin 
         implements RecordWisePlugin
{
    int nrow = 13;
    int nvars = 5;

    public RandomPlugin(String fileName, String modeName) {
	System.err.println("[RandomPlugin] " + fileName + "  (" + modeName +")");
    }

    public int getNumRecords() { 
	return(nrow);
    }

    public int getNumVariables() { 
	return(nvars);
    }

    public String[] getVariableNames() {
	String[] varNames = new String[nvars];
        for(int i = 0; i < nvars; i++) {
	    varNames[i] = "Var" + (i+1);
	}
	return(varNames);
    }

    public double[] getRecord(int which) {
      double[] vals = new double[nvars];
      for(int i = 0; i < nvars; i++) {
        vals[i] = Math.random() * (i+1) + i;
      }
      System.err.println("Getting record " + which);
      return(vals);
    }

    public String getSourceDescription(boolean brief) {
	if(brief)
            return("jrandom");
        else
	    return("Random data generator via Java");
    }
}
