/* Implement GGobi.Surface with a Clutter.Actor
*/

public abstract class GGobi.SurfaceClutter : Clutter.Actor, Surface {
  
  /* The parent */
  public Surface parent {
    get {
      Clutter.Actor actor = get_parent();
      if (actor is Surface)
        return actor as Surface;
      else return null;
    }
    construct { set_parent(value); }
  }

  /* Physical manifestation */
  public Viewport viewport {
    get {
      Clutter.Geometry geometry = get_geometry();
      return new Viewport { geometry.x, geometry.y,
          geometry.width, geometry.height };
    }
  }

  /* Events */

  public bool sensitive { /* can receive events */
    get { return get_reactive(); }
    construct set { set_reactive(value); }
  }

  public void grab_key_focus() {
    get_stage().set_key_focus(this);
  }
  public void has_key_focus() {
    get_stage().get_key_focus() == this;
  }
  
  private static ButtonEvent convert_button_event(Clutter.ButtonEvent ce) {
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

  private static MotionEvent convert_motion_event(Clutter.MotionEvent ce) {
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
    e.key = unicode_value;
    e.modifiers = (Modifiers)ce.modifier_state;
    return e;
  }
  
  private void on_button_press(Clutter.ButtonEvent e) {
    button_press(convert_button_event(e));
  }
  private void on_button_release(Clutter.ButtonEvent e) {
    button_release(convert_button_event(e));
  }

  private void on_motion(Clutter.MotionEvent e) {
    pointer_move(convert_motion_event(e));
  }

  private void on_key_press(Clutter.KeyEvent e) {
    key_press(convert_key_event(e));
  }
  private void on_key_release(Clutter.KeyEvent e) {
    key_release(convert_key_event(e));
  }
  
  construct {
    button_press_event += on_button_press;
    button_release_event += on_button_release;
    motion_event += on_motion;
    key_press_event += on_key_press;
    key_release_event += on_key_release;
  }
}