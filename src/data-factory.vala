using GLib;
using Gsf;
using Xml;

// FIXME: make this an interface?
public abstract class GGobi.DataFactory : GLib.Object {
  
  /**
   * create:
   * @self: a #GGobiDataFactory
   * @source: the source from which to retrieve the data
   *
   * Creates a list of #GGobiData objects from the input source. This means that
   * multiple datasets may exist at the source, such as in a GGobi XML file.
   *
   * Returns: #GSList of #GGobiData objects
   */
  public virtual SList<Data>?
  create(InputSource source)
  {
    Input input;

    try {
      input = source.get_input();
    } catch(GLib.Error err) {
      return null; // FIXME: should not catch this error (propagate)
    }

    // FIXME: InputSource.get_input should really throw an error here.
    if (input == null)
      return null;
    
    //input = decode_input(input);
    
    SList<Data> datasets = create_for_input(input);
    foreach(weak Data dataset in datasets) {
      if (dataset.name == null)
        dataset.name = source.display_name;
      dataset.source = source;
    }
    
    return datasets;
  }
  
  public abstract SList<Data> create_for_input(Input input);
  
  private Input
  decode_input(Input input)
  {
    return Input.uncompress(input);
  }
  
  private static int
  is_ext(string ext, string path)
  {
    string dot_ext = ".".concat(ext);
    return (int)(!path.has_suffix(dot_ext)); /* 0 means found */
  }
  
  /**
   * supports_source:
   * @self: a #GGobiDataFactory
   * @source: the source to check for compatibility with this factory
   *
   * This checks whether this factory can successfully create a #GGobiData
   * from the input source. This includes checking whether the
   * format of the data makes sense to this factory.
   *
   * Returns: TRUE if this factory supports the input source.
   */
  public virtual bool
  supports_source(InputSource source) {
    string mode = source.mode;
    bool supports = false;
    if (mode != null)
      supports = get_supported_modes().find_custom(mode, strcmp) != null;
    else { /* check file extensions for NULL mode */
      string uri = source.uri;
      URI parsed_uri = URI.parse(uri);
      if (parsed_uri != null && parsed_uri.path != null) {
        uint len, read;
        SList<string> exts = get_file_exts_for_mode(mode);
        /* assume extensions are unescaped UTF8 */
        string unescaped = URI.unescape_string(parsed_uri.path, 0, null);
        string utf8_path = unescaped.locale_to_utf8(-1, out read, out len, null);
        if (utf8_path != null) {
          string path = utf8_path.casefold();
          supports = exts.find_custom(path, (GLib.CompareFunc)is_ext) != null;
        } else warning("Could not convert path '%s' to UTF8", unescaped);
      }
      if (parsed_uri == null)
        warning("Could not parse URI: %s", uri);
    }
    return supports;
  }
  
  /**
   * get_supported_modes:
   * @self: a #GGobiDataFactory
   *
   * Lists the supported logical "modes" for this factory. Modes are character
   * strings that should uniquely identify a mode or format of data. If the
   * client is aware of the data mode, it does not have to rely on heuristics
   * like file extensions and other magic when finding a compatible factory.
   *
   * Returns: #GSList of mode names
   */
  public abstract SList<string>
  get_supported_modes();
  
  /**
   * get_file_exts_for_mode:
   * @self: a #GGobiDataFactory
   * @mode: a string identifier of a logical data mode
   *
   * Lists the file extensions that correspond to a given mode. This is useful
   * for filtering in file browsers, when the desired mode is known, and for
   * heuristically searching for a compatible factory based on the file extension.
   * This method differs from ggobi_data_factory_get_base_file_exts_for_mode() in
   * that this method also returns extensions concatenated with extensions
   * for supported encodings (such as .gz for gzip compression).
   *
   * Returns: #GSList of file extension strings
   */
  public SList<string>
  get_file_exts_for_mode(string? mode)
  {
    SList<string> decoded_exts = new SList<string>();
    
    decoded_exts.prepend("gz".casefold());
    decoded_exts.prepend("bz2".casefold());
    
    SList<string> exts = new SList<string>();
    SList<string> factory_exts = get_base_file_exts_for_mode(mode);
    foreach(string ext in factory_exts) {
      exts.append(ext);
      foreach (string d_ext in decoded_exts)
        exts.append(ext.concat(".", d_ext));
    }
    
    return exts;
  }
  
  /**
   * get_base_file_exts_for_mode:
   * @self: a #GGobiDataFactory
   * @mode: a string identifier of a logical data mode 
   *
   * Lists the file extensions that correspond to a given mode. Normally you
   * should use ggobi_data_factory_get_file_exts_for_mode() above, because
   * it also considers input decoders.
   *
   * Returns: #GSList of file extension strings
   */
  public abstract SList<string>
  get_base_file_exts_for_mode(string? mode);
}
