/* 
= GUI for Jitter stage =

Exposes:

  * overall distribution type: random/uniform
  * amount of jitter for each variable 0 - 1

*/

using GLib;
using Gtk;

public class GGobi.GuiJitter : Window {
  /* Link to model = jitter stage */
  public StageJitter stage { construct; get; }

  private HScale amount;
  private ComboBox dist;
  private Varlist varlist;
  
  construct {
    title = "Jittering";
    set_default_size(300, 250);
    set_border_width(2);
    create_widgets();
  }
  
  public GuiJitter(construct StageJitter stage) {}
  
  public void create_widgets() {

    // Initialize variable list 
    varlist = new Varlist(stage, new FilterAttributes());
    varlist.selection_changed += varlist => {
      update_amount_slider();
    };

    varlist.add_col(typeof(string), "Jittering", new VariableJitter());
    stage.col_parameter_changed += (stage, col) => { 
      varlist.update_col(2); 
    };

    Label distLabel = new Label("Distribution:");
    dist = new ComboBox.text();
    dist.append_text("Uniform");
    dist.append_text("Normal");
    // dist.set_tooltip_text("The jittering is either distributed uniformly or normally");
    dist.active = 0;
    dist.changed += dist => {
      stage.uniformDist = (dist.get_active() == 0);
      stage.refresh();
    };
    
    Label amountLabel = new Label("Amount of jitter:");
    amount = new HScale.with_range(0, 1, 0.01);
    amount.value_changed += amount => { 
      stage.update_amounts(varlist.selected_vars(), amount.get_value()); 
    };
    
    Button rejitter = new Button.with_label("Rejitter");
    rejitter.clicked += rejitter => {
      stage.refresh();
    };

    // Layout -----------------------------------------
    HBox sides = new HBox(false, 3);
    
    VBox left = new VBox(true, 5);
    left.pack_start(varlist.get_ui(), true, true, 0);
    sides.pack_start(left, true, true, 2);
    
    VBox right = new VBox(false, 5);    
    right.pack_start(distLabel, false, false, 0);
    right.pack_start(dist, false, false, 0);
    right.pack_start(amountLabel, false, false, 0);
    right.pack_start(amount, false, false, 0);
    right.pack_end(rejitter, false, false, 0);
    
    sides.pack_start(right, false, false, 2);
    
    add(sides);
    show_all();    
  }
    
  private void update_amount_slider() {
    SList<uint> selected = varlist.selected_vars();
    if (selected.length() == 0) return;
    amount.set_value(stage.amount[(int) selected.data]);
  }
  
}

public class GGobi.VariableJitter : VariableDescription {
  override string describe(Stage stage, uint j) {
    return ((StageJitter) stage).amount[j].to_string("%0.2f");
  }
}
