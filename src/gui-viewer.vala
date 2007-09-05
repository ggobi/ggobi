/* 
Provide a very simple spreadsheet-like view of a stage, particularly useful
for debugging the stages.

Long term this should probably become a custom TreeModel decorating a stage, 
but this will work for now.
*/
using GLib;
using Gtk;

public class GGobi.GuiViewer : Window {
  private Stage _stage;
  public Stage stage; // { 
  //     get;
  //     set { 
  //       _stage = value;
  //       initialise_model();
  //     }
  //   }
  public TreeView table;
  public ListStore model;
  
  construct {
    title = "Data viewer";
    set_default_size(200, 300);
    
    create_widgets ();
  }

  public Viewer.new_with_stage(GGobi.Stage stage) {}

  public void create_widgets () {
    initialise_model();
    
    
    ScrolledWindow scroll = new ScrolledWindow(null, null);
    scroll.add(table);
    scroll.show_all();
    
    add(scroll);
    show_all();
  }

  public void load_data() {
    TreeIter iter;

    for(uint i = 0; i < stage.n_rows; i++) {
      model.append(out iter);
      model.set(out iter, 0, stage.get_row_id(i));
      for(uint j = 0; j < stage.n_cols; j++) {
        model.set(out iter, j + 1, stage.get_raw_value(i, j));
      }
    }
  }
  
  public void initialise_model() {
    if (stage == null) return;
    
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
      col_types[j + 1]  = typeof(double);
      col_labels[j + 1] = v.name;
    }
    
    model = new ListStore.newv((int) ncols, col_types);
    // TreeModel sorted = new TreeModel.with_model(model);

    load_data();
    
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
    table.headers_clickable = true;
    
  }
}