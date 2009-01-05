/* 
= Variable list widget =

This variable list widget provides a common interface to display lists of 
variable names (and optional extra information) manage selection, and 
sorting/searching etc.

Currently supports one extra column of string info, but that's clearly got to
be fixed.

*/

using Gtk;
using GLib;

public class GGobi.Varlist : GLib.Object {
  public Stage stage { construct; get; }
  
  public VariableFilter filter { construct; get; }
  public TreeView vartable;
  public ListStore vars;
  
  private uint _columns = 0;
  private GLib.Type[] _coltypes;
  private string[] _titles;
  private VariableDescription[] _desc;
  
  private bool[] _excluded;

  construct {
    add_col(typeof(uint), "#", null);
    add_col(typeof(string), "Variable", new VariableName());
    
    _excluded.resize((int) stage.n_cols);
    for(uint j = 0; j < stage.n_cols; j++) {
      Variable v = stage.get_variable(j);      
      _excluded[j] = filter.exclude(v);
    }
  }

  public signal void selection_changed();

  public Varlist(Stage stage, VariableFilter filter) {
    this.stage = stage;
    this.filter = filter;
  }

  public void build() {
    vars = new ListStore.newv(_coltypes);

    add_rows();
    update_cols();
    
    vartable = new TreeView.with_model(vars);
    vartable.rules_hint = true;
    vartable.enable_search = true;
    vartable.search_column = 1;

    add_view_cols();
    
    // Allow multiple selection
    TreeSelection sel = vartable.get_selection();
    sel.set_mode(SelectionMode.MULTIPLE);
    sel.changed += sel => {
      selection_changed();
    };
  }
  
  public Widget get_ui() {
    build();
    ScrolledWindow scroll = new ScrolledWindow(null, null);
    scroll.add(vartable);
    scroll.set_policy(PolicyType.AUTOMATIC, PolicyType.AUTOMATIC);
    return scroll;
  }
  
  public uint add_col(GLib.Type type, string title, VariableDescription? desc) {
    _columns++;
    _coltypes.resize((int) _columns);
    _coltypes[(int) _columns - 1] = type;

    _titles.resize((int) _columns);
    _titles[(int) _columns - 1] = title;

    _desc.resize((int) _columns);
    _desc[(int) _columns - 1] = desc;
    
    return _columns - 1;
  }
  
  // Add all columns to view
  private void add_view_cols() {
    for(int k = 0; k < _columns; k++) add_view_col(k);    
  }
  
  private void add_view_col(int position) {
    TreeViewColumn col = new TreeViewColumn();
    col.title = _titles[position];
    CellRenderer renderer = new CellRendererText();
    col.pack_start(renderer, true);
    col.add_attribute(renderer, "text", position);
    col.set_sort_column_id(position);
    vartable.append_column(col);
  }

  // Add rows to varlist and initialize first column
  private void add_rows() {
    TreeIter iter;
    for(uint j = 0; j < stage.n_cols; j++) {
      if (_excluded[j]) continue;

      vars.append(out iter);
      vars.set(iter, 0, j);
    }    
  }

  // Update all columns of varlist
  // First column is fixed to be variable position and is not updated
  public void update_cols() {
    for(uint k = 1; k < _columns; k++) update_col(k);
  }
  
  public void update_col(uint col) {
    TreeIter iter;

    vars.get_iter_first(out iter);
    for(uint j = 0; j < stage.n_cols; j++) {
      if (_excluded[j]) continue;

      vars.set(iter, col, _desc[col].describe(stage, j));
      vars.iter_next(ref iter);
    }
  }
  
  /* Compute which variables are currently selected */
  public SList<uint> selected_vars() {
    SList<uint> selected = new SList<uint>();

    TreeSelection sel = vartable.get_selection();
    TreeIter iter;

    vars.get_iter_first(out iter);
    for(uint j = 0; j < stage.n_cols; j++) {
      if (sel.iter_is_selected(iter)) {
        selected.append(j);
      }
      vars.iter_next(ref iter);
    }
    
    return selected;
  }  
}