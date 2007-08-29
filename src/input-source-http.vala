using GLib;
using Gsf;

public class GGobi.InputSourceHTTP : InputSource {
  public string uri { get; set construct; }
  public string mode { get; set construct; }
  public string display_name { get; set construct; }
  
  public Input
  get_input(out GLib.Error err) 
  {
    InputHTTP input = new InputHTTP(uri);
    if (mode == null)
      mode = input.content_type;
    return input;
  }
  
  public InputSourceHTTP(construct string uri) { }
}
