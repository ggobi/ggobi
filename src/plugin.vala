using GLib;

public abstract class GGobi.Plugin : TypeModule {
  public PluginDescription description { get; construct; }
  public void load_dependencies() {
    /* Load any plugins on which this one depends. Make certain they 
       are fully loaded and initialized. Potential for inter-dependencies
       that would make this an infinite loop. Hope the user doesn't get this
       wrong as there are no checks at present.
    */
    if (description.has_dependencies()) {
      SList<string> deps = description.get_dependencies();
      foreach (string dep in deps) {
        PluginDescription dep_desc;
        // TODO: uncomment after GGobiApplication exists
        /*
        dep_desc =
        GGobiApplication.get_instance().get_plugin_description(dep);
        */
        Plugin dep_plugin = Plugin.create(dep_desc);
        dep_plugin.use();
        dep_plugin.unuse();
      }
    }
  }
  
  public static Plugin create(PluginDescription desc) {
    return (Plugin)Object.new(desc.plugin_type, "description", desc);
  }
}