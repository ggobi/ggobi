/* Implement GGobi.SurfaceBuffer using a ClutterTexture */

public class GGobi.SurfaceBufferClutter : SurfaceClutter, Surface,
  SurfaceBuffer
{
  private bool dirty; /* do we need to redraw? */
  
  private Clutter.Texture texture_actor = new Clutter.Texture();
  private Cogl.Handle fbo;

  public SurfaceBufferClutter(Surface parent) {
    this.parent = parent;
  }
  
  private void on_size_change(Object obj, ParamSpec pspec) {
    resize();
  }
  
  construct {
    /* Connect up any signals which could change our underlying size */
    notify["width"] += on_size_change;
    notify["height"] += on_size_change;
    notify["scale-x"] += on_size_change;
    notify["scale-y"] += on_size_change;
    notify["rotation-angle-x"] += on_size_change;
    notify["rotation-angle-y"] += on_size_change;
    notify["rotation-angle-z"] += on_size_change;
    resize();
  }

  private static Drawable drawable = new DrawableCogl();
  
  private void resize() {
    /* create a Cogl texture */
    Cogl.Handle tex = new Cogl.Texture.with_size(viewport.width,
                                                 viewport.height, 32,
                                                 false, PixelFormat.RGBA_8888);
    /* hand it to the actor for drawing */
    texture_actor.set_cogl_texture(tex);
    /* cache a framebuffer object to direct drawing to the texture */
    fbo = new Cogl.Offscreen.to_texture(tex);
    /* force a redraw */
    repaint(drawable);
  }

  /* For saving texture memory */
  override void realize() {
    texture_actor.realize();
  }
  override void unrealize() {
    texture_actor.unrealize();
  }
  
  /* Drawing */
  
  override void paint() {
    if (dirty) {
      /* Redirect output to our texture */
      Cogl.draw_buffer(Cogl.BufferTarget.OFFSCREEN_BUFFER, fbo);
      Cogl.paint_init(new Clutter.Color { 0, 0, 0, 0 }); /* clear */
      /* Render out actor scene to fbo */
      paint_buffer();
      /* Restore drawing to the frame buffer */
      Cogl.draw_buffer(Cogl.BufferTarget.WINDOW_BUFFER, COGL_INVALID_HANDLE);
      dirty = false;
    }
    texture_actor.paint();
  }

  public void repaint() {
    dirty = true;
    queue_redraw();
  }
}