/* Implement Drawable with Cogl and OpenGL */

using GL;
using Pango;

public class GGobi.DrawableCogl : Drawable {

  public int width { get; construct; }
  public int height { get; construct; }

  construct {
    Enable(VERTEX_ARRAY | LINE_SMOOTH);
  }
  
  public DrawableCogl(int width, int height) {
    this.width = width;
    this.height = height;
  }
  
  /* if there is a stroke and fill, but they're not equal, need outline */
  private bool need_outline;
  
  private Color stroke;
  private Color fill;
  
  /* configure colors */

  private void set_color(Color color) {
    Color4f(color.red, color.green, color.blue, color.alpha);
  }
  
  private void check_need_outline() {
    if ((stroke != null && fill != null && !stroke.equal(fill))) {
      need_outline = true;
    } else need_outline = false;
  }
  
  public void set_stroke_color(Color color) {
    stroke = color;
    check_need_outline();
    if (fill == null)
      PolygonMode(FRONT_AND_BACK, LINE);
    set_color(stroke); /* stroke is the default color */
  }
  public void set_fill_color(Color? color) {
    fill = color;
    if (fill != null)
      PolygonMode(FRONT_AND_BACK, FILL);
    else PolygonMode(FRONT_AND_BACK, LINE);
    check_need_outline();
  }
  
  /* configure line parameters */
  public void set_line_width(int width) {
    LineWidth(width);
  }
  // NOTE: this is very approximate
  public void set_dashes(int[] dashes, int offset) {
    ushort pattern = 0;
    int i, dashed = 0;
    if (dashes.length == 0) {
      Disable(LINE_STIPPLE);
      return;
    }
    for (i = dashes.length - 1; dashed < 16; i--) {
      if (i < 0)
        i = dashes.length - 1;
      pattern << dashes[i];
      if (i % 2 == 0) { /* on bits */
        ushort mask = ushort.MAX;
        mask << dashes[i];
        pattern |= ~mask;
      }
      dashes += dashes[i];
    }
    mask = ushort.MAX;
    mask << offset;
    mask = ~mask & pattern;
    pattern >> offset;
    pattern |= mask << (16 - offset);
    Enable(LINE_STIPPLE);
    LineStipple(1, pattern);
  }

  /* note that setting line join, cap and miter not supported */

  /* set clip */
  public void set_clip(int x, int y, int width, int height) {
    Cogl.clip_set(x, y, width, height);
  }
  public void unset_clip() {
    Cogl.clip_unset();
  }
  
  /* draw stuff */
  public void draw_rectangle(int x, int y, int width, int height) {
    //g_debug("drawing rect: %d %d %d %d", x, y, width, height);
    // FIXME: direct use of glRecti() would be more efficient...
    vertices(POLYGON,
             new int[] { x, x, x + width, x + width },
             new int[] { y, y + height, y, y + height });
  }
  
  public void draw_circle(int x, int y, int r) {
    float cx = x, cy = y;
    int[] vx = new int[r*4];
    int[] vy = new int[r*4];
    float step = Math.PI / (2*r);
    int i = 0;
    for(float theta = 0; theta < Math.PI/2; i += 4, theta += step) {
      float xmod = (Math.cos(theta) * radius);
      float ymod = (Math.sin(theta) * radius);
      vx[i] = vx[i+1] = cx + xmod;
      vy[i] = vy[i+2] = cy - ymod;
      vy[i+1] = vy[i+3] = cy + ymod;
      vx[i+2] = vx[i+3] = cx - xmod;
    }
    vertices(POLYGON, x, y);
  }
  public void draw_polyline(int[] x, int[] y) {
    vertices(LINE_STRIP, x, y);
  }
  public void draw_polygon(int[] x, int[] y) {
    vertices(POLYGON, x, y);   
  }

  /* fast paths */
  public void draw_points(int[] x, int[] y) {
    vertices(POINTS, x, y);
  }
  public void draw_segments(int[] x, int[] y) {
    vertices(LINES, x, y);
  }

  private static PangoClutterFontMap font_map = new PangoClutterFontMap();
  
  private Context pango_context = font_map.create_context();
  
  /* configure font */
  /* these will be set on PangoContext */
  public void set_font_family(string family) {
    FontDescription desc = pango_context.get_font_description();
    desc.set_family(family);
  }
  public void set_font_size(int size) {
    FontDescription desc = pango_context.get_font_description();
    desc.set_size(PANGO_SCALE*size);
  }
  public void set_font_style(FontStyle style) {
    FontDescription desc = pango_context.get_font_description();
    PangoWeight pango_style;
    switch(style) {
    case FontWeight.OBLIQUE:
      pango_weight = PangoWeight.OBLIQUE;
      break;
    case FontWeight.ITALIC:
      pango_weight = PangoWeight.ITALIC;
      break;
    default:
      pango_weight = PangoWeight.NORMAL;
      break;
    }
    desc.set_style((PangoStyle)style);
  }
  public void set_font_weight(FontWeight weight) {
    FontDescription desc = pango_context.get_font_description();
    PangoWeight pango_weight;
    switch(weight) {
    case FontWeight.BOLD:
      pango_weight = PangoWeight.BOLD;
      break;
    default:
      pango_weight = PangoWeight.NORMAL;
      break;
    }
    desc.set_weight(pango_weight);
  }

  /* querying font */
  public void font_extents(out int ascent, out int descent) {
    FontDescription desc = pango_context.get_font_description();
    FontMetrics metrics = pango_context.get_metrics(desc, null);
    ascent = metrics.get_ascent();
    descent = metrics.get_descent();
  }
  public void text_extents(out int width, out int height) {
    Layout layout = layout_text(str);
    Rectangle extents;
    layout.get_line(0).get_pixel_extents(null, out extents);
    width = extents.width;
    height = extents.height;
  }

  /* draw text */

  public void set_text_rotation(double rot) {
    Pango.Matrix matrix = new Pango.Matrix() { 1., 0., 0., 1., 0., 0. };
    matrix.rotate(rot);
    pango_context.set_matrix(matrix);
  }
  
  public void draw_text(string str, int x, int y) {
    /* using a PangoLayout */
    Layout layout = layout_text(str);
    PangoClutter.render_layout(layout, x, y, stroke, 0);
  }

  public void draw_glyphs(string glyph, int[] x, int[] y) {
    Layout layout = layout_text(glyph);
    Clutter.Color text_color = to_clutter_color(stroke);
    /* get glyph out of layout, lookup texture in cache, draw it all over */
    // NOTE: this is pretty inefficient, but we lack low-level access
    // to the clutter glyph cache. Might need to roll our own.
    for (int i = 0; i < x.length; i++)
      pango_clutter_render_layout(layout, x[i], y[i], text_color, 0);
  }

  private Layout layout_text(string str) {
    Layout layout = new Layout(pango_context);
    layout.set_text(str);
    return layout;
  }
  
  private void vertices(GLenum mode, int[] x, int[] y) {
    if (stroke == null)
      return;
    int[] vertices = new int[x.length*2];
    for (int i = 0; i < x.length; i++) {
      vertices[i*2] = x[i];
      vertices[i*2+1] = y[i];
    }
    VertexPointer(2, INT, 0, vertices);
    if (need_outline)
      set_color(fill);
    DrawArrays(mode, 0, x.length);
    if (need_outline) {
      PolygonMode(FRONT, LINE);
      set_color(stroke);
      DrawArrays(mode, 0, x.length);
      PolygonMode(FRONT, FILL);
    }
  }
}