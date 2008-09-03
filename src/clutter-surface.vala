/* Implement GGobi.Surface with a Clutter.Actor
*/

public class GGobi.Surface.ClutterSurface : Clutter.Actor, Surface {
  
  /* The parent */
  public Surface? parent {
    get {
      Clutter.Actor actor = get_parent();
      if (actor is Surface)
        return actor as Surface;
      else return null;
    }
  }

  /* Physical manifestation */
  public Viewport get_viewport() {
    Clutter.Geometry geometry;
    get_geometry(out geometry);
    return Viewport() { x = geometry.x, y = geometry.y,
        width = geometry.width, height = geometry.height };
  }

  public bool shown {
    get { return visible; }
    set construct { visible = value; }
  }
  
  /* Events */

  public bool sensitive { /* can receive events */
    get { return get_reactive(); }
    construct set { set_reactive(value); }
  }

  public void grab_key_focus() {
    ((Clutter.Stage)get_stage()).set_key_focus(this);
  }
  public void has_key_focus() {
    ((Clutter.Stage)get_stage()).get_key_focus() == this;
  }
  
  private static ButtonEvent
  convert_button_event(Clutter.ButtonEvent ce) {
    ButtonEvent e = new ButtonEvent();
    e.type = ce.type == Clutter.EventType.BUTTON_PRESS ?
      EventType.BUTTON_PRESS : EventType.BUTTON_RELEASE;
    e.x = ce.x;
    e.y = ce.y;
    e.modifiers = (Modifiers)ce.modifier_state;
    e.button = ce.button;
    e.n_clicks = ce.click_count;
    return e;
  }

  private static MotionEvent
  convert_motion_event(Clutter.MotionEvent ce) {
    MotionEvent e = new MotionEvent();
    e.type = EventType.MOTION;
    e.x = ce.x;
    e.y = ce.y;
    e.modifiers = (Modifiers)ce.modifier_state;
    return e;
  }
  
  private static KeyEvent convert_key_event(Clutter.KeyEvent ce) {
    KeyEvent e = new KeyEvent();
    e.type = ce.type == Clutter.EventType.KEY_PRESS ?
      EventType.KEY_PRESS : EventType.KEY_RELEASE;
    e.key = ce.unicode_value;
    e.modifiers = (Modifiers)ce.modifier_state;
    return e;
  }

  // VALABUG: does not check signatures very well here
  // 1) named 'surface' as 'self' but did not report conflict
  // 2) had 'ClutterSurface' as 'Surface' but shouldn't it be 'Clutter.Actor'?
  private bool on_button_press(ClutterSurface surface, Clutter.ButtonEvent e) {
    return button_press(convert_button_event(e));
  }
  private bool on_button_release(ClutterSurface surface, Clutter.ButtonEvent e)
  {
    return button_release(convert_button_event(e));
  }

  private bool on_motion(ClutterSurface surface, Clutter.MotionEvent e) {
    return pointer_move(convert_motion_event(e));
  }

  private bool on_key_press(ClutterSurface surface, Clutter.KeyEvent e) {
    return key_press(convert_key_event(e));
  }
  private bool on_key_release(ClutterSurface surface, Clutter.KeyEvent e) {
    return key_release(convert_key_event(e));
  }
  
  construct {
    button_press_event += on_button_press;
    button_release_event += on_button_release;
    motion_event += on_motion;
    key_press_event += on_key_press;
    key_release_event += on_key_release;
  }

  public void destroy() {
    base.destroy();
  }
  
}