/* 
Provide a very simple spreadsheet-like view of a stage, particularly useful
for debugging the stages.
*/
using GLib;
using Gtk;
using GGobi;

public class Viewer : Window {
  public GGobi.Stage stage; // { 
   //    get;
   //    set { 
   //      _stage = stage;
   //      stage.col_data_changed += (j) => {load_column(j);};
   //    }
   //  };
  public TreeView table;
  
  construct {
    title = "Data viewer";
    set_default_size(400, 600);
    
    create_widgets ();
  }

  public Viewer.new_with_stage(GGobi.Stage stage) {}

  public void create_widgets () {
  }

  public void load_data() {
 
  }
  
  public void load_column(uint j) {
    
  }
  
  public void initialise_model() {
    uint ncols = stage.n_cols + 1;
    
    GLib.Type[] col_types = new GLib.Type[ncols];
    string[] col_labels = new string[ncols];

    col_types[0] = typeof(string);
    col_labels[0] = "Row Label";

    for(uint j = 0; j < stage.n_cols; j++) {
      Variable v = stage.get_variable(j);
      switch (v.vartype) {
        case VariableType.INTEGER: col_types[j+1] = typeof(int); break;
        case VariableType.CATEGORICAL: col_types[j+1] = typeof(string); break;
        default: col_types[j+1] = typeof(double); break;
      }
      col_labels[j+1] = v.name;
    }
    
    ListStore model = new ListStore((int) ncols, col_types);
    // TreeModel sorted = new TreeModel.with_model(model);
    TreeView view = new TreeView.with_model(model);
    
    view.set_headers_visible(true);
    view.set_headers_clickable(true);
  }
}