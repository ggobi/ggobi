using GLib;

public abstract class GGobi.Transform : Object {

  /**
   * forward:
   * @vals: the raw values to transform
   * @var: the variable with metadata (statistics) about the values
   *
   * Performs the (forward) transformation of the values. Raw to Transformed.
   *
   * Returns: the transformed values
   */
  public abstract double[]? forward(double[] vals, Variable v);
  
  /**
   * reverse:
   * @vals: the transformed values to reverse transform
   * @var: the variable with metadata (statistics) about the values
   *
   * Performs the reverse transformation of the values. Transformed to Raw.
   *
   * Returns: the reverse transformed values.
   */
  public abstract double[]? reverse(double[] vals, Variable v);
  
  /* Transformed variable name (to indicate the current transformation) */
  public abstract string variable_name(string name);
  
  public signal void notify();
  
  /**
   * Convenience method for transforming a column in a #GGobiStage.
   * Returns transformed values for column @j starting from @first_row in @stage.
   */
  public double[] column(Stage stage, uint first_row, uint j)
  {
    uint n = stage.n_rows - first_row;
    Variable v = stage.get_variable(j);
    double[] vals = new double[n];
    
    for (uint i = first_row; i < stage.n_rows; i++)
      vals[i] = stage.get_raw_value(i, j);
    
    return forward(vals, v);
  }
  
  /* Name of this transform.  */
  public abstract string get_name();
  
  /* Description of this transform. */
  public abstract string get_description();
  
  /**
   * compare:
   * @other: the transform to which this one is compared
   *
   * This checks whether this transform has the same behavior as @other.
   * By default this just checks the GType.
   *
   * Returns: %TRUE if the two transforms have the same behavior.
   */
  public virtual bool compare(Transform other)
  {
    /* I wanted to do a property-based comparison here, but GValue
       apparently does not support comparisons */
     return this.get_type() == other.get_type();
  }
}
