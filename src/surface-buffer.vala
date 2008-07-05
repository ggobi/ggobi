/* A leaf surface that caches its drawing in a buffer */

/* When the size of the surface changes, the buffer is resized and
   marked dirty. Whenever the buffer is dirty, whether through a
   change in size or a through a call to 'repaint()', the
   'paint_buffer' signal is emitted, informing clients to redraw the
   buffer.
*/

public interface GGobi.SurfaceBuffer : Surface {
  public abstract void repaint(); /* force a redraw */
  /* redraw the dirty buffer */
  public signal void paint_buffer(Drawable drawable); 
}