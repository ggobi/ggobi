/* Implement Drawable with Cogl and OpenGL */

/* Cogl is an abstraction over OpenGL and OpenGL ES. The abstraction
   is not useful to GGobi, as it does not target embedded
   systems. However, Cogl does offer some utilities on top of OpenGL,
   although most of them are aimed at eye candy apps, like media
   centers. Currently, Cogl is used for clipping and text rendering
   (via ClutterPango).
*/

/* Ideas:
   - occlusion query to test whether a glyph needs to be drawn (1.5),
     probably a lot more work than it saves
   - VBO for caching, instead of display lists (1.5), probably no benefit
   - point sprites for faster drawing of textures (1.5)
     http://www.informit.com/articles/article.aspx?p=770639&seqNum=7
*/

using GL;
using Pango;

public class GGobi.Surface.CoglDrawable : Drawable, Object {
  
  public uint width { get; set construct; }
  public uint height { get; set construct; }

  private GLuint fc_dl;
  private GLuint oc_dl;

  private Cogl.Handle clutter_texture;
  private GLuint dummy_texture; 
  
  construct {
    /* TOOD: save state here */
    EnableClientState(Consts.VERTEX_ARRAY);
    Enable(Consts.TEXTURE_2D);
    /* OGL 1.4 */
    /* The ordinary BlendFunc() applies the same blending to the
       alpha channel as the RGB channels. The below emulates Cairo. */
    BlendFuncSeparate(Consts.SRC_ALPHA, Consts.ONE_MINUS_SRC_ALPHA,
                      Consts.ONE, Consts.ONE_MINUS_SRC_ALPHA);
    clutter_texture = Cogl.Texture.new_from_file("/usr/share/gtk-2.0/demo/apple-red.png", 0, false, Cogl.PixelFormat.ANY);
    Cogl.Texture.get_gl_texture(clutter_texture, out dummy_texture, null);
    BindTexture(Consts.TEXTURE_2D, dummy_texture);
    Enable(Consts.POINT_SPRITE);
    TexEnvi(Consts.POINT_SPRITE, Consts.COORD_REPLACE, 1);
    PointSize(10);
    PointParameteri(Consts.POINT_SPRITE_COORD_ORIGIN, Consts.LOWER_LEFT);
    //Enable(Consts.POINT_SMOOTH);
  }
  
  public override void finalize() {
    /* restore state here */
    base.finalize();
  }
  
  public CoglDrawable(uint width, uint height) {
    this.width = width;
    this.height = height;
  }

  private Color stroke;
  private Color fill;
  
  /* configure colors */

  private void set_color(Color color) {
    Color4f(color.red, color.green, color.blue, color.alpha);
  }
  
  public void set_stroke(Color? color) {
    /* cannot set 'stroke' to NULL, as it is non-nullable. We could
       make it nullable, but then it would be dynamically allocated in
       C. To avoid that overhead, we set alpha to 0 to indicate null. */
    if (color == null)
      stroke.alpha = 0;
    else stroke = color;
  }
  public void set_fill(Color? color) {
    if (color == null)
      fill.alpha = 0;
    else fill = color;
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
  // NOTE: not sure if this will work, since it probably uses the
  // stencil buffer, which has issues with FBOs
  public void set_clip(int x, int y, uint width, uint height) {
    Cogl.Clip.set(x, y, (int)width, (int)height);
  }
  public void unset_clip() {
    Cogl.Clip.unset();
  }
  
  /* draw stuff */
  public void draw_rectangle(int x, int y, uint width, uint height) {
    //g_debug("drawing rect: %d %d %d %d", x, y, width, height);
    if (fill.alpha > 0)
      Recti((GLint)(x - 1), (GLint)(y - 1), (GLint)(x + width - 2),
            (GLint)(y + height - 2));
    if (stroke.alpha > 0)
      vertices(Consts.LINE_LOOP,
               new int[] { x, x, x + (int)width, x + (int)width },
               new int[] { y, y + (int)height, y, y + (int)height });
  }

  public void draw_circle(int cx, int cy, uint r) {
    //FIXME: obviously need to setup the transformation matrix here if
    //we are going to use display lists.
    /*    
    if (fill.alpha > 0) {
      set_color(fill);
      if (fc_dl == 0) {
        fc_dl = midpoint_circle(cx, cy, r - 1, true);
      }
      CallList(fc_dl);
    }
    if (stroke.alpha > 0) {
      set_color(stroke);
      midpoint_circle(cx, cy, r, false);
    }
    */
    if (fill.alpha > 0) {
      set_color(fill);
      //  Cogl.Program.use(fc_program);
      draw_dummy_texture(cx, cy, r - 1);
    }
    if (stroke.alpha > 0) {
      set_color(stroke);
      //Cogl.Program.use(oc_program);
      //int loc = Cogl.Program.get_uniform_location(oc_program, "halfwidth");
      //float halfwidth = 1/(4.0f*r);
      //Cogl.Program.uniform_1f(loc, halfwidth);
      draw_dummy_texture(cx, cy, r);
    }
    
    //Cogl.Program.use(Cogl.Handle.INVALID);

  }

  private void draw_dummy_texture(int cx, int cy, uint r) {
    /*
      Cogl.Texture.rectangle(clutter_texture,
                           Clutter.Fixed.from_int((int)(cx - r)),
                           Clutter.Fixed.from_int((int)(cy - r)),
                           Clutter.Fixed.from_int((int)(cx + r)),
                           Clutter.Fixed.from_int((int)(cy + r)),
                           Clutter.Fixed.from_int(0),
                           Clutter.Fixed.from_int(0),
                           Clutter.Fixed.from_float(1),
                           Clutter.Fixed.from_float(1));
    */
    
    //BindTexture(Consts.TEXTURE_2D, dummy_texture);
    /*
    Begin(Consts.QUADS);
    TexCoord2f(0, 0);
    Vertex2f(cx - r, cx - r);
    TexCoord2f(1, 0);
    Vertex2f(cx + r, cy - r);
    TexCoord2f(1, 1);
    Vertex2f(cx + r, cy + r);
    TexCoord2f(0, 1);
    Vertex2f(cx - r, cy + r);
    End();
    */
    
    int x = 300000;
    /*
    float[] v = new float[x*2];

    for (int i = 0; i < x*2; i+=2) {
      v[i] = cx;
      v[i+1] = cy; // + 400*(i/300000.0f);
    }
    VertexPointer(2, Consts.FLOAT, 0, v);
    DrawArrays(Consts.POINTS, 0, (GLsizei)(v.length/2.0));
*/
    Begin(Consts.POINTS);
    for (int i = 0; i < x; i++) {
      Vertex2f(cx, cy);
    }
    End();
  }

  private GLuint midpoint_circle(int cx, int cy, uint r, bool fill) {
    /* midpoint circle algorithm, implementation adapted from:
       http://en.wikipedia.org/wiki/Midpoint_circle_algorithm */
    /* TODO: some way to guess final value of 'x' to allocate an array */
    
    int f = 1 - (int)r;
    int ddF_x = 1;
    int ddF_y = -2 * (int)r;
    int x = 0;
    int y = (int)r, fy;
    int tx = 0;
    int[] v = null;
    int j = 0;

    if (fill) { // not clear why this correction is needed
      cy++;
    }
      
    while(x < y) {  /* need to preallocate array */
      if(f >= 0) {
        y--;
        ddF_y += 2;
        f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;
    }

    if (!fill) {
      tx = 2*x;
      v = new int[8*(tx+1)];
      v[0] = cx; v[1] = cy + (int)r;
      v[2] = cx; v[3] = cy - (int)r;
      v[4] = cx + (int)r; v[5] = cy;
      v[6] = cx - (int)r; v[7] = cy;
      j = 8;
    }

    if (fill) {
      v = new int[8*x + 8*(r - y + 1) + 4];
      v[v.length - 4] = cx - (int)r; v[v.length - 3] = cy;
      v[v.length - 2] = cx + (int)r + 1; v[v.length - 1] = cy;
    }

    y = (int)r;
    f = 1 - (int)r;
    ddF_x = 1;
    ddF_y = -2 * (int)r;
    x = 0;
    
    while(x < y) {
      if(f >= 0) {
        if (fill) {
          v[j++] = cx - x; v[j++] = cy + y;
          v[j++] = cx + x + 1; v[j++] = cy + y;
          v[j++] = cx - x; v[j++] = cy - y;
          v[j++] = cx + x + 1; v[j++] = cy - y;
        }
        y--;
        fy--;
        ddF_y += 2;
        f += ddF_y;
      }
      x++; // not clear why 'fill' correction needed here
      v[j++] = cx + y + (int)fill; v[j++] = cy + x;
      v[j++] = cx - y; v[j++] = cy + x;
      v[j++] = cx + y + (int)fill; v[j++] = cy - x;
      v[j++] = cx - y; v[j++] = cy - x;
      if (!fill) {
        v[j++] = cx - x; v[j++] = cy + y;
        v[j++] = cx + x; v[j++] = cy + y;
        v[j++] = cx - x; v[j++] = cy - y;
        v[j++] = cx + x; v[j++] = cy - y;
      }
      ddF_x += 2;
      f += ddF_x;
    }

    if (r > 0 && fill) {
      v[j++] = cx - x; v[j++] = cy + y;
      v[j++] = cx + x; v[j++] = cy + y;
      v[j++] = cx - x; v[j++] = cy - y;
      v[j++] = cx + x; v[j++] = cy - y;
    }
    
    VertexPointer(2, Consts.INT, 0, v);

    GLuint dl = GenLists(1);
    NewList(dl, Consts.COMPILE);
    if (fill)
      DrawArrays(Consts.LINES, 0, (GLsizei)(v.length/2.0));
    else DrawArrays(Consts.POINTS, 0, (GLsizei)(v.length/2.0));
    EndList();
    
    return(dl);
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
  private static Pango.FontMap font_map = new Pango.Clutter.FontMap();
  
  private Context pango_context =
    ((Pango.Clutter.FontMap)font_map).create_context();
  
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
    Pango.Matrix matrix = Pango.Matrix() { xx=1, xy=0, yx=0, yy=1, x0=0, y0=0 };
    matrix.rotate(rot);
    pango_context.set_matrix(matrix);
  }
  
  public void draw_text(string str, int x, int y) {
    /* using a PangoLayout */
    if (fill.alpha > 0) {
      Layout layout = layout_text(str);
      Pango.Clutter.render_layout(layout, x, y, to_clutter_color(fill), 0);
    }
  }

  private Clutter.Color to_clutter_color(Color color) {
    Clutter.Color clutter = Clutter.Color();
    clutter.red = (uchar) (color.red * 255);
    clutter.blue = (uchar) (color.blue * 255);
    clutter.green = (uchar) (color.green * 255);
    clutter.alpha = (uchar) (color.alpha * 255);
    return clutter;
  }

  /* Perhaps there is too much overhead to texture mapping. Consider
     VBOs or display lists (probably better supported and a bit
     faster). Just translate the transformation matrix and call list. */
  public void draw_glyphs(string glyph, int[] x, int[] y) {
    if (fill.alpha == 0)
      return;
    Layout layout = layout_text(glyph);
    Clutter.Color text_color = to_clutter_color(fill);
    /* get glyph out of layout, lookup texture in cache, draw it all over */
    // NOTE: this is pretty inefficient, but we lack low-level access
    // to the clutter glyph cache. One idea is to cheat and get the
    // PangoClutterRenderer from the PangoClutterFontMap and use it.
    // GOOD NEWS -- Clutter now has official API for getting its
    // PangoRenderer, so this issue can now be resolved for Clutter 1.0
    
    for (int i = 0; i < x.length; i++)
      Pango.Clutter.render_layout(layout, x[i], y[i], text_color, 0);
  }

  private Layout layout_text(string str) {
    Layout layout = new Layout(pango_context);
    layout.set_text(str, -1);
    return layout;
  }
  
  private void vertices(GLenum mode, int[] x, int[] y)
  {
    int[] vertices = new int[x.length*2];
    for (int i = 0; i < x.length; i++) {
      vertices[i*2] = x[i];
      vertices[i*2+1] = y[i];
    }
    VertexPointer(2, Consts.INT, 0, vertices);
    if (mode == Consts.POLYGON) {
      if (fill.alpha > 0) {
        set_color(fill);
        DrawArrays(Consts.POLYGON, 0, (GLsizei)(x.length));
      }
      if (stroke.alpha > 0)
        mode = Consts.LINE_LOOP;
    }
    if (mode != Consts.POLYGON) {
      set_color(stroke);
      DrawArrays(mode, 0, (GLsizei)(x.length));
    }
  }
}