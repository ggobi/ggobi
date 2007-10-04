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
  
  public TreeView vartable;
  public ListStore vars;

  public signal void selection_changed();

  public Varlist(construct Stage stage) {}

  construct {
    vars = new ListStore(3, typeof(uint), typeof(string), typeof(string));

    // Add variables to list store
    for(uint j = 0; j < stage.n_cols; j++) {
      TreeIter iter;

      vars.append(out iter);
      vars.set(out iter, 0, j);
      vars.set(out iter, 1, stage.get_variable(j).name);
    }
    
    vartable = new TreeView.with_model(vars);

    // Add columns to view
    add_view_col("#", 0);
    add_view_col("Variable", 1);
    vartable.rules_hint = true;
    
    // Allow multiple selection
    TreeSelection sel = vartable.get_selection();
    sel.set_mode(SelectionMode.MULTIPLE);

    sel.changed += sel => {
      selection_changed();
    };
  }
  
  public Widget get_ui() {
    ScrolledWindow scroll = new ScrolledWindow(null, null);
    scroll.add(vartable);
    scroll.set_policy(PolicyType.AUTOMATIC, PolicyType.AUTOMATIC);
    return scroll;
  }
  
  public void add_view_col(string title, int position) {
    TreeViewColumn col = new TreeViewColumn();
    col.title = title;
    CellRenderer renderer = new CellRendererText();
    col.pack_start(renderer, true);
    col.add_attribute(renderer, "text", position);
    col.set_sort_column_id(position);
    vartable.append_column(col);
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