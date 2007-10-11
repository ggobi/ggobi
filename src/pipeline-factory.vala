using GLib;

public class GGobi.PipelineFactory : Object {
  /**
   * build:
   * @root: the root stage (dataset) of the nascent pipeline
   *
   * Creates a pipeline given a root stage. The logic of creating stages,
   * configuring them and connecting them is implemented by signal handlers
   * connected to the 'build' signal.
   * 
   * By connecting to the 'build' signal, it is easy to insert a custom
   * pipeline stage at any point in the pipeline, whenever a pipeline
   * is created using this factory.
   *
   */
  public signal void build(Stage root);
  
}
