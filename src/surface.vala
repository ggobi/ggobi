/*
   A surface performs two primary roles:
   1) a drawing target
   2) a source of user events

   Everything is in screen coordinates.

   Surfaces can be combined in SurfaceContainers to form a scene
   graph.
   
   The drawing actually occurs in the leaves at SurfaceBuffers.
   
   If an event occurs within the region of a sensitive surface (or if
   there is a key event and it has focus), the corresponding signal is
   emitted.
*/

//VALABUG: need to define these before the Surface interface
namespace GGobi.Surface {

  public enum Modifiers
  {
    SHIFT_MASK = 1 << 1,
    LOCK_MASK = 1 << 2,
    CONTROL_MASK = 1 << 3,
    MOD1_MASK = 1 << 4,
    MOD2_MASK = 1 << 5,
    MOD3_MASK = 1 << 6,
    MOD4_MASK = 1 << 7,
    MOD5_MASK = 1 << 8,
    BUTTON1_MASK = 1 << 9,
    BUTTON2_MASK = 1 << 10,
    BUTTON3_MASK = 1 << 11,
    BUTTON4_MASK = 1 << 12,
    BUTTON5_MASK = 1 << 13
  }

  public enum EventType {
    KEY_PRESS,
    KEY_RELEASE,
    MOTION,
    BUTTON_PRESS,
    BUTTON_RELEASE
  }

  public struct Viewport {
    public int x;
    public int y;
    public uint width;
    public uint height;
  }

  public class Event {
    public EventType type;
  }
    
  public class PointerEvent : Event {
    public int x;
    public int y;
    public Modifiers modifiers;
  }

  public class MotionEvent : PointerEvent { }

  public class ButtonEvent : PointerEvent
  {
    public uint button;
    public uint n_clicks;
  }
  
  public class KeyEvent : Event
  {
    public unichar key;
    public Modifiers modifiers;
  }

}

public interface GGobi.Surface.Surface : Object {

  /* The parent */
  public abstract Surface? parent { get; construct set; }

  /* Physical manifestation */
  /* VALABUG: 'visible' renamed to 'shown' due to conflicts with clutter */
  public abstract bool shown { get; construct set; }
  public abstract Viewport viewport { get; }
  
  /* Destroy surface (remove from container) */
  public abstract void destroy();
  
  /* Events */

  /* can receive events */
  public abstract bool sensitive { get; construct set; }

  /* only one surface can have the key focus */
  public abstract void grab_key_focus();
  public abstract void has_key_focus();
  
  public signal bool button_press(ButtonEvent e);
  public signal bool button_release(ButtonEvent e);
  public signal bool pointer_move(MotionEvent e);
  public signal bool key_press(KeyEvent e);
  public signal bool key_release(KeyEvent e);
}
