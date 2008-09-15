/* Implement Drawable with Cogl and OpenGL */
 
using GL;
using Pango;

public class GGobi.Surface.CoglDrawable : Drawable, Object {
  
  public uint width { get; set construct; }
  public uint height { get; set construct; }

  construct {
    /* save state here */
    EnableClientState(Consts.VERTEX_ARRAY);
    Enable(Consts.LINE_SMOOTH);
  }

  public override void finalize() {
    /* restore state here */
    base.finalize();
  }
  
  public CoglDrawable(uint width, uint height) {
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
    if ((stroke.alpha != 0 && fill.alpha != 0 && !stroke.equal(fill))) {
      need_outline = true;
    } else need_outline = false;
  }
  
  public void set_stroke(Color color) {
    stroke = color;
    check_need_outline();
    if (fill.alpha == 0)
      PolygonMode(Consts.FRONT, Consts.LINE);
    set_color(stroke); /* stroke is the default color */
  }
  public void set_fill(Color? color) {
    fill = color;
    if (fill.alpha != 0)
      PolygonMode(Consts.FRONT, Consts.FILL);
    else PolygonMode(Consts.FRONT, Consts.LINE);
    check_need_outline();
  }
  
  /* configure line parameters */
  public void set_line_width(uint width) {
    LineWidth(width);
  }
  // NOTE: this is very approximate
  public void set_dashes(uint[] dashes, uint offset) {
    ushort pattern = 0;
    uint i, dashed = 0;
    if (dashes.length == 0) {
      Disable(Consts.LINE_STIPPLE);
      return;
    }
    for (i = dashes.length - 1; dashed < 16; i--) {
      if (i < 0)
        i = dashes.length - 1;
      pattern <<= (ushort)dashes[i];
      if (i % 2 == 0) { /* on bits */
        ushort mask = ushort.MAX;
        mask <<= (ushort)dashes[i];
        pattern |= ~mask;
      }
      dashed += dashes[i];
    }
    ushort mask = ushort.MAX;
    mask <<= (ushort)offset;
    mask = ~mask & pattern;
    pattern >>= (ushort)offset;
    pattern |= mask << (16 - offset);
    Enable(Consts.LINE_STIPPLE);
    LineStipple(1, (GLushort)pattern);
  }

  /* note that setting line join, cap and miter not supported */

  /* set clip */
  // NOTE: clipping is the only part that depends on Cogl
  public void set_clip(int x, int y, uint width, uint height) {
    Cogl.Clip.set(x, y, (int)width, (int)height);
  }
  public void unset_clip() {
    Cogl.Clip.unset();
  }
  
  /* draw stuff */
  public void draw_rectangle(int x, int y, uint width, uint height) {
    //g_debug("drawing rect: %d %d %d %d", x, y, width, height);
    // FIXME: direct use of glRecti() would be more efficient...
    vertices(Consts.POLYGON,
             new int[] { x, x, x + (int)width, x + (int)width },
             new int[] { y, y + (int)height, y, y + (int)height });
  }
  
  public void draw_circle(int x, int y, uint r) {
    int[] vx = new int[r*4];
    int[] vy = new int[r*4];
    float step = (float) Math.PI / (2*r);
    int i = 0;
    for(float theta = 0; theta < Math.PI/2; i += 4, theta += step) {
      int xmod = (int)(Math.cosf(theta) * r);
      int ymod = (int)(Math.sinf(theta) * r);
      vx[i] = vx[i+1] = x + xmod;
      vy[i] = vy[i+2] = y - ymod;
      vy[i+1] = vy[i+3] = y + ymod;
      vx[i+2] = vx[i+3] = x - xmod;
    }
    vertices(Consts.POLYGON, vx, vy);
  }
  public void draw_polyline(int[] x, int[] y) {
    vertices(Consts.LINE_STRIP, x, y);
  }
  public void draw_polygon(int[] x, int[] y) {
    vertices(Consts.POLYGON, x, y);   
  }

  /* fast paths */
  public void draw_points(int[] x, int[] y) {
    vertices(Consts.POINTS, x, y);
  }
  public void draw_segments(int[] x, int[] y) {
    vertices(Consts.LINES, x, y);
  }

  /* hopefully PangoClutter stuff is temporary -
     Cairo needs a working OpenGL backend */
  private static Pango.Clutter.FontMap font_map = new Pango.Clutter.FontMap();
  
  private Context pango_context = font_map.create_context();
  
  /* configure font */
  /* these will be set on PangoContext */
  public void set_font_family(string family) {
    weak FontDescription desc = pango_context.get_font_description();
    desc.set_family(family);
  }
  public void set_font_size(uint size) {
    weak FontDescription desc = pango_context.get_font_description();
    desc.set_size(Pango.SCALE*(int)size);
  }
  public void set_font_style(FontStyle style) {
    weak FontDescription desc = pango_context.get_font_description();
    Style pango_style;
    switch(style) {
    case FontStyle.OBLIQUE:
      pango_style = Style.OBLIQUE;
      break;
    case FontStyle.ITALIC:
      pango_style = Style.ITALIC;
      break;
    default:
      pango_style = Style.NORMAL;
      break;
    }
    desc.set_style((Style)style);
  }
  public void set_font_weight(FontWeight weight) {
    weak FontDescription desc = pango_context.get_font_description();
    Weight pango_weight;
    switch(weight) {
    case FontWeight.BOLD:
      pango_weight = Weight.BOLD;
      break;
    default:
      pango_weight = Weight.NORMAL;
      break;
    }
    desc.set_weight(pango_weight);
  }

  /* querying font */
  public void font_extents(out int ascent, out int descent) {
    weak FontDescription desc = pango_context.get_font_description();
    FontMetrics metrics = pango_context.get_metrics(desc, null);
    ascent = metrics.get_ascent();
    descent = metrics.get_descent();
  }
  public void text_extents(string str, out uint width, out uint height) {
    Layout layout = layout_text(str);
    Rectangle extents;
    layout.get_line(0).get_pixel_extents(null, out extents);
    width = extents.width;
    height = extents.height;
  }

  /* draw text */

  public void set_text_rotation(double rot) {
    Pango.Matrix matrix = Pango.Matrix() { xx=1., xy=0., yx=0., yy=1., x0=0.,
                                           y0=0. };
    matrix.rotate(rot);
    pango_context.set_matrix(matrix);
  }
  
  public void draw_text(string str, int x, int y) {
    /* using a PangoLayout */
    Layout layout = layout_text(str);
    Pango.Clutter.render_layout(layout, x, y, to_clutter_color(stroke), 0);
  }

  private Clutter.Color to_clutter_color(Color stroke) {
    Clutter.Color color;
    color.red = (uchar) (stroke.red * 255);
    color.blue = (uchar) (stroke.blue * 255);
    color.green = (uchar) (stroke.green * 255);
    color.alpha = (uchar) (stroke.alpha * 255);
    return color;
  }
  
  public void draw_glyphs(string glyph, int[] x, int[] y) {
    Layout layout = layout_text(glyph);
    Clutter.Color text_color = to_clutter_color(stroke);
    /* get glyph out of layout, lookup texture in cache, draw it all over */
    // NOTE: this is pretty inefficient, but we lack low-level access
    // to the clutter glyph cache. One idea is to cheat and get the
    // PangoClutterRenderer from the PangoClutterFontMap and use it.
    for (int i = 0; i < x.length; i++)
      Pango.Clutter.render_layout(layout, x[i], y[i], text_color, 0);
  }

  private Layout layout_text(string str) {
    Layout layout = new Layout(pango_context);
    layout.set_text(str, -1);
    return layout;
  }
  
  private void vertices(GLenum mode, int[] x, int[] y) {
    if (stroke.alpha == 0)
      return;
    int[] vertices = new int[x.length*2];
    for (int i = 0; i < x.length; i++) {
      vertices[i*2] = x[i];
      vertices[i*2+1] = y[i];
    }
    VertexPointer(2, Consts.INT, 0, vertices);
    if (need_outline)
      set_color(fill);
    DrawArrays(mode, 0, (GLsizei)x.length);
    if (need_outline) {
      PolygonMode(Consts.FRONT, Consts.LINE);
      set_color(stroke);
      DrawArrays(mode, 0, (GLsizei)x.length);
      PolygonMode(Consts.FRONT, Consts.FILL);
    }
  }
}