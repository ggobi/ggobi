using GLib;
using Xml;

public class GGobi.InputSourceFactory
{
  /**
   * get_supported_schemes:
   * @self: a #GGobiInputSourceFactory
   *
   * Lists the URI schemes supported by this factory. If a
   * URI has one of these schemes, this factory can create a #GGobiInputSource
   * capable of retrieving the data.
   *
   * Returns: #GSList of scheme strings, like "http"
   */
  public virtual SList<string>
  get_supported_schemes()
  {
    SList<string> schemes = new SList<string>();
    schemes.prepend("http");
    schemes.prepend("ftp");
    schemes.prepend("file");
    schemes.prepend(null); /* unknown */
    return schemes;
  }
  
  /**
   * create:
   * @self: a #GGobiInputSourceFactory
   * @uri: a URI identifying the source of the data
   * @mode: logical mode of the data, if known, otherwise %NULL
   *
   * Creates a #GGobiInputSource that is capable of retrieving data from
   * the given URI. If the mode of the data is known, providing it will
   * set the mode on the input source, for convenience.
   *
   * Returns: #GGobiInputSource for the URI
   */
  public virtual InputSource
  create(string uri, string mode)
  {
    InputSource source = null;
    string source_type = null;
    
    URI parsed_uri = URI.parse(uri);
    if (parsed_uri == null) {
      critical("Failed to parse URI: %s", uri);
      return null;
    }
    
    /* no scheme assume file */
    if (parsed_uri.scheme == null || parsed_uri.scheme == "file")
      source = new InputSourceFile(uri);
    else if (parsed_uri.scheme == "http")
      source = new InputSourceHTTP(uri);
    else if (parsed_uri.scheme == "ftp")
      source = new InputSourceFTP(uri);
    
    if (source != null) {
      source.mode = mode;
      if (source.display_name == null)
        source.display_name = Path.get_basename(source.uri);
    }
    
    return source;
  }
}

