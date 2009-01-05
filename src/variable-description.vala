using GLib;

public abstract class GGobi.VariableDescription : Object {
  public abstract string describe(Stage stage, uint j);
}

public class GGobi.VariableName : VariableDescription {
  public override string describe(Stage stage, uint j) {
    Variable v = stage.get_variable(j);
    return v.name;
  }
}

