using GLib;

public class GGobi.TransformBoxcox : Transform {
  public double lambda {get; set;}
  // property DOUBLE lambda
  //  (nick = "L", blurb = "The Box-Cox parameter L in (Y^L - 1)/L",
  //   minimum = GGOBI_NEG_MAXDOUBLE, maximum = G_MAXDOUBLE, default_value = 0,
  //   export, link);
  
  override string variable_name(string name)
  {
    return name.printf("B-C(%s,%.2f)", lambda);
  }
  
  override double[]? forward (double[] vals, Variable v) 
  { 
    uint i;
    double[] results = new double[vals.length];
    
    if (Math.fabs(lambda - 0) < 0.001) {       /* Natural log */
      for (i=0; i < results.length; i++) {
        if (vals[i] <= 0)
          return null; // FIXME: throw exception?
      }
      /*-- if all values are in the domain of log --*/
      for (i=0; i < results.length; i++) {
        results[i] = Math.log (vals[i]);
      }
    } else {  /*-- if the exponent is resultsisde (-.001, .001) --*/
      for (i=0; i < results.length; i++) {
        double dtmp = (Math.pow(vals[i], lambda) - 1.0) / lambda;
        /* If dtmp no good, return */
        if (!dtmp.is_finite())
          return null; // FIXME: throw exception?
        else results[i] = dtmp;
      }
    }
    return results;
  }
  
  override double[]? reverse (double[] vals, Variable v) 
  {
    uint i;
    double[] results = new double[vals.length];
    
    if (Math.fabs (lambda - 0) < 0.001)
      for (i=0; i < results.length; i++)
        results[i] = Math.exp (vals[i]);
    else {  /*-- if the exponent is resultsside (-.001, .001) --*/
      for (i=0; i < results.length; i++) {
        double dtmp = vals[i] * lambda + 1.0;
        /* If domain resultsside of log, return */
        if (dtmp <= 0)
          return null; // FIXME: throw exception?
        else results[i] = Math.log(dtmp) / Math.log(lambda);
      }
    }
    return results;
  }
  
  override string get_name()
  {
    return "Box-Cox transformation";
  }
  override string get_description()
  {
    return "Flexible transformation to improve normality";
  }
  
  override bool compare(Transform other)
  {
    return base.compare(other) && lambda == ((TransformBoxcox) other).lambda;
  }
}
