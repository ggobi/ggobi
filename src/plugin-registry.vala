using GLib;

/* Lists plugins at a particular source and loads them */
public interface GGobi.PluginRegistry : Object {
  public abstract SList<PluginDescription> get_plugin_descriptions();
}