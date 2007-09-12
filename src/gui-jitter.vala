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

  public TreeView vartable;
  public ListStore vars;

  private HScale amount;
  private ComboBox dist;
  
  construct {
    title = "Jittering";
    set_default_size(300, 250);
    set_border_width(2);
    create_widgets();
  }
  
  public GuiJitter(construct StageJitter stage) {}
  
  public void create_widgets() {
    dist = new ComboBox.text();
    dist.append_text("Uniform");
    dist.append_text("Normal");
    // dist.set_tooltip_text("The jittering is either distributed uniformly or normally");
    dist.active = 0;
    dist.changed += dist => {
      stage.uniformDist = (dist.get_active() == 0);
      stage.refresh();
    };
    
    amount = new HScale.with_range(0, 1, 0.01);
    amount.value_changed += amount => { 
      stage.update_amounts(selected_vars(), amount.get_value()); 
    };
    
    Button rejitter = new Button.with_label("Rejitter");
    rejitter.clicked += rejitter => {
      stage.refresh();
    };

    initialise_varlist();
    
    /* Layout */
    HBox sides = new HBox(false, 3);
    
    VBox left = new VBox(true, 5);
    ScrolledWindow scroll = new ScrolledWindow(null, null);
    scroll.add(vartable);
    scroll.set_policy(PolicyType.AUTOMATIC, PolicyType.AUTOMATIC);
    left.pack_start(scroll, true, true, 0);
    sides.pack_start(left, true, true, 2);
    
    VBox right = new VBox(false, 5);
    // right.set_alignment(Justification.LEFT);
    Label distLabel = new Label("Distribution:");
    Label amountLabel = new Label("Amount of jitter:");
    
    right.pack_start(distLabel, false, false, 0);
    right.pack_start(dist, false, false, 0);
    right.pack_start(amountLabel, false, false, 0);
    right.pack_start(amount, false, false, 0);
    
    right.pack_end(rejitter, false, false, 0);
    
    sides.pack_start(right, false, false, 2);
    
    add(sides);
    show_all();
    
  }
  
  private void update_jittered_values() {
    TreeIter iter;
    vars.get_iter_first(out iter);
    for(uint j = 0; j < stage.n_cols; j++) {
      vars.set(out iter, 1, stage.amount[j].to_string("%0.2f"));
      vars.iter_next(out iter);
    }
  }
  
  private void update_amount_slider() {
    TreeSelection sel = vartable.get_selection();
    
    TreeIter iter;

    vars.get_iter_first(out iter);
    for(uint j = 0; j < stage.n_cols; j++) {
      if (sel.iter_is_selected(out iter)) {
        amount.set_value(stage.amount[j]);
        return;
      }
      vars.iter_next(out iter);
    }
    
    
  }
  
  private void initialise_varlist() {
    vars = new ListStore(2, typeof(string), typeof(string));

    // Add variables to list store
    for(uint j = 0; j < stage.n_cols; j++) {
      TreeIter iter;

      vars.append(out iter);
      vars.set(out iter, 0, stage.get_variable(j).name);
      vars.set(out iter, 1, stage.amount[j].to_string("%0.2f"));
    }
    
    vartable = new TreeView.with_model(vars);

    // Add column in store to view
    TreeViewColumn col = new TreeViewColumn();
    col.title = "Variable";
    CellRenderer renderer = new CellRendererText();
    col.pack_start(renderer, true);
    col.add_attribute(renderer, "text", 0);
    col.resizable = true;
    vartable.append_column(col);
    
    col = new TreeViewColumn();
    col.title = "Jittering";
    renderer = new CellRendererText();
    col.pack_start(renderer, true);
    col.add_attribute(renderer, "text", 1);
    // col.add_attribute(renderer, "width", 3);
    vartable.append_column(col);
    
    vartable.rules_hint = true;
    
    stage.amounts_changed += (stage, cols) => { 
      update_jittered_values(); 
    };
    
    // Allow multiple selection
    TreeSelection sel = vartable.get_selection();
    sel.set_mode(SelectionMode.MULTIPLE);
    // Connect to slide
    sel.changed += sel => {
      update_amount_slider();
    };
  }
  
  /* Compute which variables are currently selected */
  public SList<uint> selected_vars() {
    SList<uint> selected = new SList();

    TreeSelection sel = vartable.get_selection();
    TreeIter iter;

    vars.get_iter_first(out iter);
    for(uint j = 0; j < stage.n_cols; j++) {
      if (sel.iter_is_selected(out iter)) {
        selected.append(j);
      }
      vars.iter_next(out iter);
    }
    
    return selected;
  }
  

  
  
}