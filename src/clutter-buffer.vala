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
    notify["width"] += on_size_change;
    notify["height"] += on_size_change;
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
    Cogl.Handle tex = Cogl.Texture.new_with_size(viewport.width,
                                                 viewport.height, 32,
                                                 false,
                                                 Cogl.PixelFormat.RGBA_8888);
    /* hand it to the actor for drawing */
    texture_actor.set_cogl_texture(tex);
    /* cache a framebuffer object to direct drawing to the texture */
    fbo = Cogl.Offscreen.new_to_texture(tex);
    /* update the drawable for the new size */
    drawable = new CoglDrawable(viewport.width, viewport.height);
    /* force a redraw */
    repaint();
  }

  /* For saving texture memory */
  public override void realize() {
    texture_actor.realize();
  }
  public override void unrealize() {
    texture_actor.unrealize();
  }
  
  /* Drawing */
  
  public override void paint() {
    if (dirty) {
      /* Redirect output to our texture */
      Cogl.draw_buffer(Cogl.BufferTarget.OFFSCREEN_BUFFER, fbo);
      Cogl.paint_init(Clutter.Color() { red = 0, blue = 0, green = 0,
            alpha = 0 }); /* clear */
      /* Render out actor scene to fbo */
      paint_buffer(drawable);
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