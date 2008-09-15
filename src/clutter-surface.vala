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
    construct set { visible = value; }
  }

  /* We always try to fill our parent
     (but the container governs the layout) */

  public override void get_preferred_width(Clutter.Unit for_height,
                                           out Clutter.Unit min_width_p,
                                           out Clutter.Unit natural_width_p)
  {
    Clutter.Actor parent = get_parent();
    if (parent == null)
      base.get_preferred_width(for_height, out min_width_p,
                               out natural_width_p);
    else {
      Clutter.Unit parent_height = parent.get_heightu();
      Clutter.Unit parent_width = parent.get_widthu();
      if (for_height == -1)
        for_height = (int) parent_height;
      natural_width_p = (int)((double)parent_width/parent_height * for_height);
    }
  }
  public override void get_preferred_height(Clutter.Unit for_width,
                                            out Clutter.Unit min_height_p,
                                            out Clutter.Unit natural_height_p)
  {
    Clutter.Actor parent = get_parent();
    if (parent == null)
      base.get_preferred_height(for_width, out min_height_p,
                                out natural_height_p);
    else {
      Clutter.Unit parent_height = parent.get_heightu();
      Clutter.Unit parent_width = parent.get_widthu();
      if (for_width == -1)
        for_width = (int)parent_width;
      natural_height_p = (int)((double)parent_height/parent_width * for_width);
    }
  }

  private void on_size_change(Clutter.Actor obj, ParamSpec pspec) {
    queue_relayout();
  }
  
  public override void parent_set(Clutter.Actor? old_parent) {
    /* keeping up with parent size */
    if (old_parent != null) {
      old_parent.notify["width"] -= on_size_change;
      old_parent.notify["height"] -= on_size_change;
    }
    Clutter.Actor parent = get_parent();
    if (parent != null) {
      parent.notify["width"] += on_size_change;
      parent.notify["height"] += on_size_change;
    }
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
  public override bool button_press_event(Clutter.ButtonEvent e) {
    return button_press(convert_button_event(e));
  }
  public override bool button_release_event(Clutter.ButtonEvent e) {
    return button_release(convert_button_event(e));
  }

  public override bool motion_event(Clutter.MotionEvent e) {  
    return pointer_move(convert_motion_event(e));
  }

  public override bool key_press_event(Clutter.KeyEvent e) {
    return key_press(convert_key_event(e));
  }
  public override bool key_release_event(Clutter.KeyEvent e) {
    return key_release(convert_key_event(e));
  }

  // FIXME: need to explicitly deconstruct ourselves here?
  public void destroy() {
    base.destroy();
  }
  
}