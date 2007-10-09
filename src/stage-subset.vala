/*
 Performs filtering operations specific to the GGobi subset dialog.
 Right now the dialog acts solely as a controller, not a view, of the filter,
 so there is no need to store the high-level rules that it applies.
*/
using GLib;

public class GGobi.StageSubset : StageFilter  {
  public Select selection {construct; get;}

  public StageSubset(construct Select selection) {}

  public void reset() {
    selection = new SelectAll();
    refresh();
  }

  override void refresh_() {
    selection.select(this);
    base.refresh_();
  }

} 



