using GLib;
using Gsf;

public class GGobi.InputSourceFTP : InputSource
{
  
  override Input
  get_input(out GLib.Error error) 
  {
    Input input = new InputFTP(uri);
    return input;
  }
  
  public InputSourceFTP(construct string uri) { }
}
