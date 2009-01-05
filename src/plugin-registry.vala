using GLib;

/* Lists plugins at a particular source and loads them */
public interface GGobi.PluginRegistry : Object {
  public abstract SList<PluginDescription> get_plugin_descriptions();

  // TODO: may want a get_plugin_description(string name)

  /*
    private int
    plugin_has_name(void* plugin, void* name)
    {
    string plugin_name = ((PluginDescription)plugin).name;
    return strcmp(plugin_name, name);
    }
  */
 
}