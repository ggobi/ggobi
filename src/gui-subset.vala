/* 
= GUI for subset stage =

Subsetting operators on an entire stage, not at the varible level.
*/

using GLib;
using Gtk;

public class GGobi.GuiSubset : Window {
  public StageSubset stage { construct; get; }
  
  construct {
    title = "Subsetting";
    set_default_size(200, 250);
    set_border_width(2);
    create_widgets();
  }

  RadioButton all;
  RadioButton random;
  RadioButton block;
  RadioButton every_n;
  RadioButton label;
  
  public GuiSubset(StageSubset stage) {
    this.stage = stage;
  }
  
  public void create_widgets() {
    // Subset selection radio buttons
    all = new RadioButton.with_label(null, "All");
    all.toggled += button => {
      if (all.active) update_selection(new SelectAll());
    };
        
    random = new RadioButton.with_label_from_widget(all, "Randomly");
    random.toggled += percent => {
      if (random.active) update_selection(new SelectRandom(10));
    };

    block = new RadioButton.with_label_from_widget(all, "A block");
    block.toggled += percent => {
      if (block.active) update_selection(new SelectBlock(1, 10));
    };

    every_n = new RadioButton.with_label_from_widget(all, "Every n");
    every_n.toggled += percent => {
      if (every_n.active) update_selection(new SelectEveryN(0, 5));
    };

    label = new RadioButton.with_label_from_widget(all, "By label");
    label.toggled += percent => {
      if (label.active) update_selection(new SelectLabel());
    };

    Button refresh = new Button.with_label("Re-select");
    refresh.clicked += refresh => {
      stage.refresh();
    };
    
    // Layout -----------------------------------------
    VBox group = new VBox(false, 3);
    
    group.pack_start(all, false, false, 2);
    group.pack_start(random, false, false, 2);
    group.pack_start(block, false, false, 2);
    group.pack_start(every_n, false, false, 2);
    group.pack_start(label, false, false, 2);
    
    group.pack_end(refresh, false, false, 2);
    
    add(group);
    show_all();    
  }
  
  private void update_selection(Select selection) {
    if (stage.selection.equals(selection)) return;
    stage.selection = selection;
    stage.refresh();
  }  
}
