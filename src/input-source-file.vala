using GLib;
using Gsf;
using Xml;

public class GGobi.InputSourceFile:InputSource
{ 
  
  public string uri { get; set construct; }
  public string mode { get; set construct; }
  public string display_name { get; set construct; }
  
  /** Get the system locale filename from the URI in this source */
  public string
  get_filename()
  {
    URI parsed = new URI.parse(uri);
    string resolved = null;
    if (parsed != null && parsed.scheme == null) {
      /* we have a relative URI, resolve against current working dir file:// uri
         note that if the path is already absolute, the path doesn't change
         -- we're just trying to make a file:// uri with an absolute path,
            so that glib can give us the (locale+platform)-dependent filename
      */
      string base_path = Filename.to_uri(Environment.get_current_dir());
      resolved = (string)URI.build(uri, base_path.concat("/", null));
    } else if (parsed != null) resolved = (string)parsed.save();
    else critical("Failed to parse URI: %s", uri);
    return Filename.from_uri(resolved);
  }
  
  public Input
  get_input(out GLib.Error error) 
  {
    Input input = null;
    string filename = get_filename();
    if (filename == null) {
      critical("Cannot get filename for URI: %s", uri);
    } else input = new InputStdio(filename, out error);
    return input;
  }
  
  public InputSourceFile(construct string uri) { }
}
