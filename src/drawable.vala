
/* Graphics primitive API containing a minimal context.
*/

//VALABUG: these need to be defined prior to the Drawable interface
namespace GGobi.Surface {
/* Channel values range from 0 to 1. */
  public struct Color {
    public float red;
    public float green;
    public float blue;
    public float alpha = 1;
    
    public bool equal(Color color) {
      return red == color.red && green == color.green && blue == color.blue &&
        alpha == color.alpha;
    }
  }

  public enum LineCap {
    ROUND,
    BUTT,
    SQUARE
  }

  public enum LineJoin {
    ROUND,
    MITER,
    BEVEL
  }

  public enum FontStyle {
    NORMAL,
    OBLIQUE,
    ITALIC
  }

  public enum FontWeight {
    NORMAL,
    BOLD
  }
}

public interface GGobi.Surface.Drawable : Object {

  //VALABUG: it seems interface properties cannot be construct-only
  // We don't want the width and height to be settable after construction
  public abstract uint width { get; set construct; }
  public abstract uint height { get; set construct; }
  
  /* configure colors */
  public abstract void set_stroke(Color color);
  public abstract void set_fill(Color? color);
  
  /* configure line parameters */
  public abstract void set_line_width(uint width);
  public abstract void set_dashes(uint[] dashes, uint offset);

  /* optional line aesthetics */
  public virtual void set_line_cap(LineCap cap) { }
  public virtual void set_line_join(LineJoin join) { }
  public virtual void set_miter_limit(double limit) { }

  /* set clip */
  public abstract void set_clip(int x, int y, uint width, uint height);
  public abstract void unset_clip();
  
  /* draw stuff */
  public abstract void draw_rectangle(int x, int y, uint width, uint height);
  public abstract void draw_circle(int x, int y, uint r);
  public abstract void draw_polyline(int[] x, int[] y);
  public abstract void draw_polygon(int[] x, int[] y);
  public abstract void draw_points(int[] x, int[] y);
  public abstract void draw_segments(int[] x, int[] y);
  public virtual void draw_line(int x1, int y1, int x2, int y2) {
    draw_segments(new int[] { x1, x2 }, new int[] { y1, y2 });
  }
  
  /* configure font */
  public abstract void set_font_size(uint size);  
  public abstract void set_font_family(string family);
  public abstract void set_font_style(FontStyle style);
  public abstract void set_font_weight(FontWeight weight);
  /* for asymmetric scaling - do we really need this? */
  /*public abstract void set_font_matrix();*/

  /* querying font */
  public abstract void font_extents(out int ascent, out int descent);
  public abstract void text_extents(string str, out uint width,
                                    out uint height);

  /* draw text */
  public abstract void set_text_rotation(double rot);
  public abstract void draw_text(string str, int x, int y);
  public abstract void draw_glyphs(string str, int[] x, int[] y);
}