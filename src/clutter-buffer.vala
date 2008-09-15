/* Implement GGobi.SurfaceBuffer using a ClutterTexture */

public class GGobi.Surface.ClutterBuffer : ClutterSurface, Buffer
{
  private bool dirty; /* do we need to redraw? */
  
  private Clutter.Texture texture_actor = new Clutter.Texture();
  private Cogl.Handle fbo;

  private void on_size_change(ClutterBuffer obj, ParamSpec pspec) {
    resize();
  }
  
  construct {
    /* Connect up any signals which could change our underlying size */
    notify["allocation"] += on_size_change;
    notify["scale-x"] += on_size_change;
    notify["scale-y"] += on_size_change;
    notify["rotation-angle-x"] += on_size_change;
    notify["rotation-angle-y"] += on_size_change;
    notify["rotation-angle-z"] += on_size_change;
    resize();
  }

  private Drawable drawable;
  
  private void resize() {
    /* create a Cogl texture */
    Viewport viewport = get_viewport();
    if (viewport.width <= 0 || viewport.height <= 0)
      return; /* must have positive area to make a texture */
    Cogl.Handle tex = Cogl.Texture.new_with_size(viewport.width,
                                                 viewport.height, 32,
                                                 false,
                                                 Cogl.PixelFormat.RGBA_8888);
    /* hand it to the actor for drawing */
    texture_actor.set_cogl_texture(tex);
    Cogl.Texture.unref(tex); /* will be destroyed with texture_actor */
    /* cache a framebuffer object to direct drawing to the texture */
    if (fbo != Cogl.Handle.INVALID)
      Cogl.Offscreen.unref(fbo);
    fbo = Cogl.Offscreen.new_to_texture(tex);
    /* force a redraw */
    repaint();
  }

  public override void allocate(Clutter.ActorBox box,
                                bool absolute_origin_changed)
  {
    base.allocate(box, absolute_origin_changed);
    texture_actor.allocate(box, absolute_origin_changed);
  }
  
  /* For saving texture memory */
  public override void realize() {
    texture_actor.realize();
  }
  public override void unrealize() {
    texture_actor.unrealize();
  }
  
  /* Drawing */

  // NOTE: it's not clear how widespread FBO support is
  // We may need some sort of pixmap-based drawable as a fallback
  public override void paint() {
    if (dirty) {
      Clutter.Color black = { 0, 0, 0, 0xff };
      // FIXME: need to save and restore GL state here
      /* Redirect output to our texture */
      Cogl.draw_buffer(Cogl.BufferTarget.OFFSCREEN_BUFFER, fbo);
      /* Render out actor scene to fbo */
      Viewport viewport = get_viewport();
      paint_buffer(new CoglDrawable(viewport.width, viewport.height));
      /* Restore drawing to the frame buffer */
      Cogl.draw_buffer(Cogl.BufferTarget.WINDOW_BUFFER, Cogl.Handle.INVALID);
      dirty = false;
    }
    texture_actor.paint();
  }

  public void repaint() {
    dirty = true;
    queue_redraw();
  }
}