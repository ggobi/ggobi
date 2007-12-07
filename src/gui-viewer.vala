/* 
Provide a very simple spreadsheet-like view of a stage, particularly useful
for debugging the stages.

Long term this should probably become a custom TreeModel decorating a stage, 
but this will work for now.
*/
using GLib;
using Gtk;

public class GGobi.GuiViewer : Window {
  public Stage stage { construct; get; }
  public TreeView table;
  public ListStore model;
  
  public GuiViewer(construct Stage stage) {}
  construct {
    title = "Data viewer";
    set_default_size(300, 500);
    
    create_widgets ();
  }
  
  public void create_widgets () {
    initialize();
    load_data();
    
    stage.changed += (stage, msg) => {process_incoming(msg);};

    ScrolledWindow scroll = new ScrolledWindow(null, null);
    scroll.add(table);
    scroll.set_policy(PolicyType.AUTOMATIC, PolicyType.AUTOMATIC);
    scroll.show_all();
    
    add(scroll);
    show_all();
  }

  /* Initialize the model and table with columns loaded from the stage */
  public void initialize() {
    uint ncols = stage.n_cols + 1;
    
    GLib.Type[] col_types = new GLib.Type[ncols];
    string[] col_labels = new string[ncols];

    col_types[0] = typeof(string);
    col_labels[0] = "Row Label";

    for(uint j = 0; j < stage.n_cols; j++) {
      Variable v = stage.get_variable(j);
      // switch (v.vartype) {
      //   case VariableType.INTEGER: col_types[j+1] = typeof(int); break;
      //   case VariableType.CATEGORICAL: col_types[j+1] = typeof(string); break;
      //   default: col_types[j+1] = typeof(double); break;
      // }
      col_types[j + 1]  = typeof(string);
      col_labels[j + 1] = v.name;
    }
    
    model = new ListStore.newv((int) ncols, col_types);
    table = new TreeView.with_model(model);
    
    for(uint j = 0; j < ncols; j++) {
      CellRenderer renderer = new CellRendererText();

      TreeViewColumn col = new TreeViewColumn();
      col.title = col_labels[j];
      col.pack_start(renderer, true);
      col.add_attribute(renderer, "text", (int) j);
      col.resizable = true;

      table.append_column(col);
    }
    table.headers_visible = true;
    table.rules_hint = true;
  }

  /* Load all data from the stage, appending to current table */
  public void load_data() {
    for(uint i = 0; i < stage.n_rows; i++) {
      TreeIter iter;
      
      model.append(out iter);
      model.set(out iter, 0, stage.get_row_id(i));
      for(uint j = 0; j < stage.n_cols; j++) {
        model.set(out iter, j + 1, stage.get_raw_value(i, j).to_string("%0.2f"));
      }
    }
  }
  
  public void process_incoming(PipelineMessage msg) {
    /* If any rows or columns added or removed, 
       rebuild the table from scratch */
    if (msg.get_n_removed_rows() > 0 || msg.get_n_removed_cols() > 0 ||
        msg.get_n_added_rows() > 0   || msg.get_n_added_cols() > 0 ) {
      GLib.debug("Rebuilding table");
      initialize();
      load_data();
      return;
    }
    
    // Update changed columns
    foreach(uint col in msg.get_changed_cols()) {
      update_col(col);
    }
  }
  
  public void update_col(uint j) {
    TreeIter iter;
    model.get_iter_first(out iter);
    for(uint i = 0; i < stage.n_rows; i++) {
      model.set(out iter, j + 1, stage.get_raw_value(i, j).to_string("%0.2f"));
      model.iter_next(out iter);
    }
  }

}