/* A surface that contains other surfaces, forming a scene graph */

/* Child surfaces represent some subregion of their parent and
   can overlap.  */

public interface GGobi.Surface.Container : Surface {
  /* Containers have children */
  public abstract List<Surface> get_children();
}