using GLib;

public class GGobi.TransformAbs : Transform {
  
  public override string variable_name(string name)
  {
    return name.printf("abs(%s)");
  }
  
  public override double[]? forward (double[] vals, Variable v) 
  { 
    double[] results = new double[vals.length];
    for (uint i = 0; i < results.length; i++)
      results[i] = Math.fabs(vals[i]);
    return results;
  }
  
  /* we just use identity transformation for the reverse */
  public override double[]? reverse (double[] vals, Variable v) 
  {
    double[] results = new double[vals.length];
    for (uint i = 0; i < results.length; i++)
      results[i] = vals[i];
    return results;
  }
  
  public override string get_name()
  {
    return "Absolute value";
  }
  public override string get_description()
  {
    return "Take the absolute value (|x|)";
  }
}
