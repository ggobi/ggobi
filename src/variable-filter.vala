/* 
= Variable filters =

These facilitate filtering the display of varibles for various tasks.  Ideally
this would be performed with a delegate, but vala does not yet supported 
delegate instance variables.

This is an implementation of the strategy design pattern.

*/

using GLib;

public abstract class GGobi.VariableFilter : Object {
  public abstract bool exclude(Variable v);
}

public class GGobi.FilterNone : VariableFilter {
  public override bool exclude(Variable v) {
    return false;
  }
}

public class GGobi.FilterAll : VariableFilter {
  public override bool exclude(Variable v) {
    return true;
  }
}

public class GGobi.FilterAttributes : VariableFilter {
  public override bool exclude(Variable v) {
    return v.is_attribute;
  }
}

public class GGobi.FilterContinuous : VariableFilter {
  public override bool exclude(Variable v) {
    return v.vartype != VariableType.CATEGORICAL;
  }
}

public class GGobi.FilterMissing : VariableFilter {
  public override bool exclude(Variable v) {
    return !v.has_missings();
  }
}
