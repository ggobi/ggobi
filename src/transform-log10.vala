using GLib;

public class GGobi.TransformLog10 : Transform {
  
  override string variable_name(string name)
  {
    return name.printf("log10(%s)");
  }
  
  override double[] forward(double[] vals, Variable v) 
  { 
    uint i;
    double[] results = new double[vals.length];
    for (i=0; i < results.length; i++)
      if (vals[i] <= 0) return null;

    for (i=0; i < results.length; i++)
      results[i] = Math.log10(vals[i]);
    return results;
  }
  
  override double[] reverse(double[] vals, Variable v) 
  {
    double[] results = new double[vals.length];
    for (uint i=0; i < results.length; i++)
      results[i] = Math.pow(10, vals[i]);
    return results;
  }
  
  override string get_name()
  {
    return "Log, base 10";
  }

  override string get_description()
  {
    return "Take the base 10 logarithm of each value";
  }
}
