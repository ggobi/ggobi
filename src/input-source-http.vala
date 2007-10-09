using GLib;
using Gsf;

public class GGobi.InputSourceHTTP : InputSource {
  
  override Input
  get_input(out GLib.Error err) 
  {
    InputHTTP input = new InputHTTP(uri);
    if (mode == null)
      mode = input.content_type;
    return input;
  }
  
  public InputSourceHTTP(construct string uri) { }
}
