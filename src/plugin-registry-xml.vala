using GLib;
using Xml;

public class GGobi.PluginRegistryXml : PluginRegistry, Object {

  /* where to retrieve the XML - can be a directory */
  public File file { get; construct; }

  public PluginRegistryXml(File file) {
    this.file = file;
  }
  
  public SList<PluginDescription> get_plugin_descriptions() {
    /* just parse the XML on the fly for now */
    SList<PluginDescription> descs = new SList<PluginDescription>();
    PluginDescription desc;
    try {
      FileInfo root_info = file.query_info(FILE_ATTRIBUTE_STANDARD_TYPE,
                                           FileQueryInfoFlags.NONE, null);    
      if (root_info.get_file_type() == FileType.DIRECTORY) {
        FileInfo info;
        FileEnumerator file_enum;
        file_enum = file.enumerate_children(FILE_ATTRIBUTE_STANDARD_NAME,
                                            FileQueryInfoFlags.NONE, null);
        while((info = file_enum.next_file(null)) != null) {
          File child = file.get_child(info.get_name());
          desc = parse_description_file(child);
          if (desc != null)
            descs.prepend(desc);
        }
      } else {
        desc = parse_description_file(file);
        if (desc != null)
          descs.prepend(desc);
      }
    } catch(GLib.Error error) {
      critical("Failed to access path: %s", file.get_uri());
    }
    return descs;
  }

  // FIXME: may be better to throw exceptions rather than return
  // 'null' or 'false' in all of these functions
  
  private PluginDescription? parse_description_file(File file) {
    Xml.Doc* doc = Parser.parse_file(file.get_uri());
    PluginDescription desc = parse_description_doc(doc);
    if (desc == null)
      critical("Failed to parse plugin file '%s'", file.get_uri());
    return desc;
  }
  
  /**
   * create:
   * @node: libxml2 node
   * @doc: libxml2 doc
   *
   * Marshals a #GGobiPlugin from an XML description. By default, assumes
   * GGobi plugin XML format. Extending the XML format may be useful
   * for describing plugins written in other languages, for example.
   *
   * Returns: new #GGobiPlugin
   */
  public virtual PluginDescription? parse_description_doc(Xml.Doc *doc)
  {
    Xml.Node* node = get_element(doc->get_root_element(), "plugin");
    PluginDescription desc = null;
    if (node == null)
      critical("Failed to find 'plugin' node in document");
    else desc = parse_plugin(node, doc); /* metadata */
    return (desc);
  }
  
  /**
     Pick up the names of all the plugins on which this one depends.
     Then when we load this plugin, we will ensure that those plugins
     are also loaded.
  */
  private bool
  parse_dependencies(Xml.Node* node, Xml.Doc* doc, PluginDescription plugin)
  {
    Xml.Node* c, el;
    c = get_element(node, "dependencies");
    if (c == null)
      return(false);

    el = c->children;
    while (el != null) {
      if (strcmp(el->name, "dependency") == 0) {
        string val = el->get_prop("name");
        if (val != null) {
          plugin.add_dependency(val);
        }
      }
      el = el->next;
    }
    
    return(true);
  }
 
  private int
  plugin_has_name(void* plugin, void* name)
  {
    string plugin_name = ((PluginDescription)plugin).name;
    return strcmp(plugin_name, name);
  }
 
  private PluginDescription
  parse_plugin(Xml.Node* node, Xml.Doc* doc)
  {
    PluginDescription desc;
    desc = new PluginDescription(node->get_prop("name"),
                                 node->get_prop("author"),
                                 node->get_prop("description"),
                                 Type.from_name(node->get_prop("type")),
                                 node->get_prop("uri"));
    parse_dependencies(node, doc, desc); /* dependencies */
    return desc;
  }

  private Xml.Node* get_element(Xml.Node *node, string name) {
    if (strcmp (node->name, name) == 0)
      return (node);
    node = node->children;
    while (node != null) {
      if (strcmp(node->name, name) == 0) {
        return (node);
      }
      node = node->next;
    }
    return (node);
  }
}