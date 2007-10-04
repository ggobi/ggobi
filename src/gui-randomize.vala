/* 
= GUI for imputation stage =


*/

using GLib;
using Gtk;

public class GGobi.GuiRandomize : Window {
  public StageRandomize stage { construct; get; }
  private Varlist varlist;
  RadioButton none;
  RadioButton repeat;
  RadioButton unique;
  
  construct {
    title = "Randomization";
    set_default_size(400, 250);
    set_border_width(2);
    create_widgets();
  }
  
  public GuiRandomize(construct StageRandomize stage) {}
  
  public void create_widgets() {
    varlist = new Varlist(stage);
    varlist.selection_changed += varlist => {
      update_randomization_gui();
    };
    
    update_randomization_col();
    stage.col_parameter_changed += (stage, j) => { 
      update_randomization_col();
    };
    varlist.add_view_col("Randomisation", 2);

    // Imputation selection radio buttons
    none = new RadioButton.with_label(null, "None");
    none.toggled += fixed => {
      update_randomization(RandomizationType.NONE);
    };
    
    repeat = new RadioButton.with_label(null, "With duplicates");
    repeat.group = none.group;
    repeat.toggled += percent => {
      update_randomization(RandomizationType.REPEAT);
    };
    
    unique = new RadioButton.with_label(null, "Without duplicates");
    unique.group = none.group;
    unique.toggled += percent => {
      update_randomization(RandomizationType.UNIQUE);
    };

    Button rejitter = new Button.with_label("Re-randomise");
    rejitter.clicked += rejitter => {
      stage.refresh();
    };
    
    // Layout -----------------------------------------
    HBox sides = new HBox(false, 3);
    
    VBox left = new VBox(true, 5);
    left.pack_start(varlist.get_ui(), true, true, 0);
    sides.pack_start(left, true, true, 2);
    
    VBox right = new VBox(false, 5);    
    
    right.pack_start(none, false, false, 2);
    right.pack_start(repeat, false, false, 2);
    right.pack_start(unique, false, false, 2);
    right.pack_end(rejitter, false, false, 0);
    
    sides.pack_start(right, false, false, 2);
    
    add(sides);
    show_all();    
  }
  
  private void update_randomization(RandomizationType value) {
    SList<uint> selected = varlist.selected_vars();
    foreach(uint j in selected) {
      stage.set_randomization(j, value);
    }
  }
  
  private void update_randomization_col() {
    ListStore vars = varlist.vars;
    TreeIter iter;

    vars.get_iter_first(out iter);
    for(uint j = 0; j < stage.n_cols; j++) {
      string label;
      switch (stage.randomization[j]) {
        case RandomizationType.NONE: label = "None"; break;
        case RandomizationType.REPEAT: label = "Duplicates"; break;
        case RandomizationType.UNIQUE: label = "No duplicates"; break;
      }
      vars.set(out iter, 2, label);
      vars.iter_next(out iter);
    }
  }
  
  private void update_randomization_gui() {
    SList<uint> selected = varlist.selected_vars();
    RandomizationType cur = stage.randomization[(int) selected.data];
    
    switch (cur) {
      case RandomizationType.NONE: none.active = true; break;
      case RandomizationType.REPEAT: repeat.active = true; break;
      case RandomizationType.UNIQUE: unique.active = true; break;
    }
  }
  
}