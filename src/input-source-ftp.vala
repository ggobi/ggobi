using GLib;
using Gsf;

public class GGobi.InputSourceFTP : Object, InputSource
{
  
  public string uri { get; set construct; }
  public string mode { get; set construct; }
  public string display_name { get; set construct; }
  
  public Input
  get_input(out GLib.Error error) 
  {
    Input input = new InputFTP(uri);
    return input;
  }
  
  public InputSourceFTP(construct string uri) { }
}
