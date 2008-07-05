
/* Design considerations:
   1) Graphics context is embedded within the drawable, so we don't
   have to interpret the context with every operation.
   2) No paths/transformations -- too much overhead.
   2) Everything is integers, not doubles, because we are targeting
   the screen. This isn't a big restriction.
   3) No high-level structures like 'Point' or 'Segment', wasteful.
   4) Draw circles instead of general arcs. This isn't a geometric
   drawing application.
   5) Include draw_segments fast path for drawing lots of lines.
   6) Support pattern caching for drawing many similar glyphs.
*/
public interface GGobi.Drawable {

  /* configure color */
  public abstract void set_color(Color color);

  /* configure line parameters */
  public abstract void set_line_width(int width);
  public abstract void set_line_cap(LineCap cap);
  public abstract void set_line_join(LineJoin join);
  public abstract void set_miter_limit(double limit);
  public abstract void set_dashes(int[] dashes, int offset);

  /* set clip */
  public abstract void set_clip(int x0, int x1, int y0, int y1);
  
  /* draw stuff */
  public abstract void draw_rectangle(int x, int y, int width, int height, 
                                      bool filled);
  public abstract void draw_circle(int x, int y, int r);
  public abstract void draw_line(int x1, int y1, int x2, int y2);
  public abstract void draw_polyline(int[] x, int[] y);
  public abstract void draw_polygon(int[] x, int[] y, bool filled);

  /* fast paths */
  public abstract void draw_points(int[] x, int[] y);
  public abstract void draw_segments(int[] x, int[] y);
  
  /* text handling */
  public abstract void draw_text(int x, int y, string str, double rot);
  public abstract void font_extents(out double ascent, out double descent);
  public abstract void text_extents(out double width, out double height);

  /* get size */
  public abstract void get_size(out int width, out int height);

  /* patterns */
  public abstract void start_pattern();
  public abstract Pattern finish_pattern();
  public abstract void draw_pattern(Pattern pattern, int[] x, int[] y);
}

public struct Pattern : uint;

public struct Color {
  public int red;
  public int green;
  public int blue;
  public int alpha;
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