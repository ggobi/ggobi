using GLib;

public class GGobi.PluginDll : Plugin {

  private Module module;
  
  private bool
  get_symbol(string symbol_name, out void* symbol)
  {
    return module.symbol(symbol_name, out symbol);
  }

  private delegate bool OnLoadFunc();
  private delegate bool OnUnloadFunc();
    
  public override bool load() 
  {
    load_dependencies();

    // FIXME: plugins are libraries, not data files
    string filename = find_data_file(description.uri);
    if (filename != null) {
      module = Module.open(filename, ModuleFlags.BIND_LAZY);
      if (module == null)
        critical("Error on loading plugin module %s: %s",
                 filename, Module.error());
    } else critical("Could not find dll '%s'", description.uri);

    if (module != null) {
      void *function;
      if (get_symbol("ggobi_plugin_on_load", out function)) {
        OnLoadFunc on_load = (OnLoadFunc)function;
        on_load();
      } else {
        critical("error loading plugin at %s: %s", description.uri,
                 Module.error());
        return(false);
      }
      return(true);
    }
    
    return (false);
  }
  
  public override void unload() {
    if (module != null) {
      void *function;
      if (get_symbol("ggobi_plugin_on_unload", out function)) {
        OnUnloadFunc on_unload = (OnUnloadFunc)function;
        on_unload();
      } else {
        critical("error unloading plugin %s: %s", description.name,
                 Module.error());
      }
      module = null;
    }
  }
}