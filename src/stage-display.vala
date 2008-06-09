/* 
= Display stage =

Currently in charge of connection between pipeline and the rest of GGobi.  
Eventually will become the connection to displays.

*/

using GLib;

public class GGobi.StageDisplay : Stage {

  override void process_outgoing(PipelineMessage msg)  {
    base.process_outgoing(msg);
    
    // FIXME: common method for this and PipelineMatrix.process_message
    SList<uint> removed_rows = msg.get_removed_rows();
    SList<uint> changed_cols = msg.get_changed_cols();
    uint n_added_cols = msg.get_n_added_cols();
    uint n_added_rows = msg.get_n_added_rows();
    uint n_refresh = n_added_cols;
    
    if (removed_rows != null || n_added_rows > 0) {
      // if rows changed, update all columns
      n_refresh = n_cols; 
    } else {
      foreach(uint j in changed_cols) refresh_col_(j);
    }
    for (uint j = n_cols - n_refresh; j < n_cols; j++)
      refresh_col_(j);
      
    if (gg != null && n_cols > 0) {
      displays_tailpipe (RedrawStyle.FULL, gg);
    }
  }
  
  override void refresh_col_(uint j) {
    tform_to_world_by_var(this, j);    
  }  

}
