using GLib;
using Xml;

/** General notes on GGobi I/O framework (for lack of better place):
    Right now, when loading a file, we:
    1) use InputSourceFactory to get an InputSource for a URI scheme
    2) find a DataFactory that supports the InputMode of the source
    3) get the Data from the InputSource using DataFactory
    
    One thing that is missing here is the decoding (like
    decompression) of the input. We used to have an InputDecoder, but
    I scrapped it for simplicity and just put a call to
    gsf_input_uncompress() in DataFactory. But that's gone now too,
    because it didn't quite fit.
    
    I'm tired of thinking about this, so let's move to GIO (in GLib).

    InputSource -> GFile

    This is relatively straightforward.
    
    InputSourceFactory --> GVfs
    
    GIO ships with one GVfs implementation for local files.
    There are OS-specific implementations of GVfs. Well, only one
    right now - for Linux. It supports pretty much any transport mode
    you would want, including HTTP and compressed archives. The
    question is, do we just leave it to the OS or do we provide a
    custom GVfs using e.g. libgsf.

    DataFactory -> DataFactory

    There is a GAppInfo that looks for applications that support a
    particular type of file. This does not quite fit for DataFactory,
    but we'll need to take a closer look. At least for inspiration.

    InputMode -> GContentType

    Pretty much a no-brainer.

    GsfInput -> GInputStream

    GInputStream can be retrieved from GFile. I think the libgsf
    dependency is still worth it - it supports XML input/output and
    CSV output. For stuff that uses GsfInput, like the XML parser,
    libgsf (svn) provides a wrapper called GsfInputGio.
    
 */
    
public class GGobi.InputSourceFactory : GLib.Object
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
  public virtual InputSource? // FIXME: throw an exception, not null
  create(string uri, string? mode)
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
    
    if (source != null) {
      source.mode = mode;
      if (source.display_name == null)
        source.display_name = Path.get_basename(source.uri);
    }
    
    return source;
  }
}

