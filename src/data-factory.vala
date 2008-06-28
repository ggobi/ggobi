using GLib;

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
  create(File source)
  {
    FileInputStream input;

    try {
      input = source.read(null);
    } catch(GLib.Error err) {
      return null; // FIXME: should not catch this error (propagate)
    }

    SList<Data> datasets = create_for_stream(input);
    foreach(weak Data dataset in datasets) {
      if (dataset.name == null)
        dataset.name = source.get_parse_name();
      dataset.source = source;
    }
    
    return datasets;
  }

  public virtual bool supports_file(File file) throws GLib.Error {
    FileInfo info = file.query_info(FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                    FileQueryInfoFlags.NONE, null);
    string type = g_content_type_get_mime_type(info.get_content_type());
    return strcmp(mime_type, type) == 0;
  }
  
  public abstract SList<Data> create_for_stream(InputStream input);
  
  public abstract string# mime_type { get; }
}
