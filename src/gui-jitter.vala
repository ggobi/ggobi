/* 
= GUI for Jitter stage =

Exposes:

  * overall distribution type: random/uniform
  * amount of jitter for each variable 0 - 1

*/

using GLib;
using Gtk;

class GGobi:GuiJitter : Widget {
  /* Link to model = jitter stage */
  public StageJitter stage;
  private VariableList vars;
  private GtkHScale amount;
  private GtkComboBox dist;
  
  construct {
    title = "Jittering";
    set_default_size(200, 250);
    set_border_width(5);
    create_widgets();
  }
  
  
  /* How do I connect the VariableList to recieve events from the stage? */

  public void create_widgets() {
    dist = new GtkComboBox.text();
    dist.append_text("Uniform");
    dist.append_text("Normal");
    dist.set_tool_tips("The jittering is either distributed uniformly or normally");
    dist.changed += cmb { stage.uniformDist = (cmb.get_active() == 1) }
    
    amount = new GtkHScale.new(0, 1, 0.01);
    amount.value_changed += rng { update_amounts(rng.get_value()) }
    
    vars = new VariableList.with_stage(stage);
    
    /* Layout */
    GtkVBox left, right;
    left = new GtkVBox();
    left.pack_start(vars.gui(), true, false, 0);
    add(left)
    
    right = new GtkHBox();
    right.pack_start(dist);
    right.pack_start(amount);
    add(right);
  }
  
  public void update_amounts(double amount) {
    foreach(uint i in vars.selected_indices) {
      stage.amount[i] = amount;
      stage.col_data_changed(i);
    }
    stage.flush_changes();
  }
}