public class GGobi.PluginDescription : Object {
  public string name { get; set construct; }
  public string description { get; set construct; }
  public string author { get; set construct; }
  public string uri { get; set construct; }
  public Type plugin_type { get; set construct; }

  private SList<string> dependencies;
  
  public PluginDescription(string name, string author, string description,
                           Type type, string uri) {
    this.name = name;
    this.author = author;
    this.description = description;
    this.plugin_type = type;
    this.uri = uri;
  }
      
  public bool has_dependencies() {
    return dependencies != null;
  }
  public SList<string> get_dependencies() {
    return dependencies.copy();
  }
  public void add_dependency(string name) {
    dependencies.prepend(name);
  }
}