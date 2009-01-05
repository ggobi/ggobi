/*
 Performs filtering operations specific to the GGobi subset dialog.
 Right now the dialog acts solely as a controller, not a view, of the filter,
 so there is no need to store the high-level rules that it applies.
*/
using GLib;

public class GGobi.StageSubset : StageFilter  {
  public Select selection { set construct; get;}

  public StageSubset(Select selection) {
    this.selection = selection;
  }

  construct {
    selection = new SelectAll();
  }

  public signal void subset_changed();

  public void reset() {
    selection = new SelectAll();
    refresh();
  }

  public override void refresh_() {
    selection.select(this);
    base.refresh_();
  }

} 



