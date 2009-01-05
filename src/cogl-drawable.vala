/* Implement Drawable with Cogl and OpenGL */

/* Cogl is an abstraction over OpenGL and OpenGL ES. This is not
   useful to GGobi, as it does not target embedded systems. However,
   Cogl does offer some utilities on top of OpenGL, although most of
   them are aimed at eye candy apps, like media centers. Currently,
   Cogl is used for clipping, shaders and text rendering (via
   ClutterPango).
*/
using GL;
using Pango;

public class GGobi.Surface.CoglDrawable : Drawable, Object {
  
  public uint width { get; set construct; }
  public uint height { get; set construct; }

  private Cogl.Handle dummy_texture;
  private GLuint fc_dl;
  
  construct {
    /* TOOD: save state here */
    EnableClientState(Consts.VERTEX_ARRAY);
    /* NOTE: this depends on OpenGL 1.4, which hopefully is widespread */
    /* The ordinary BlendFunc() applies the same blending to the
       alpha channel as the RGB channels. The below emulates Cairo. */
    BlendFuncSeparate(Consts.SRC_ALPHA, Consts.ONE_MINUS_SRC_ALPHA,
                      Consts.ONE, Consts.ONE_MINUS_SRC_ALPHA);
    
    dummy_texture = Cogl.Texture.new_with_size(width, height, 32, false,
                                               Cogl.PixelFormat.A_8);
  }
  
  private static Cogl.Handle fc_program;
  private static Cogl.Handle oc_program;
  
  static construct {
    Cogl.Handle frag, vert;
    
    string def_vert_shader = /* a pass-through vertex shader */
      """void main() {
	   gl_Position = ftransform();
	   gl_TexCoord[0] = gl_MultiTexCoord0;
	   gl_FrontColor = gl_Color;
      }""";

    /* NOTE: could use OpenGL directly, but it's a bit complicated,
       due to the need to access ARB extensions pre OpenGL 2.0 */

    vert = compile_shader(def_vert_shader, Cogl.Shader.Type.VERTEX_SHADER);

    /** circle drawing shaders */
    
    /* in texture coordinates, (0.5, 0.5) is center, r = 0.5 */
    /* drawing/managing a texture is more work, but works even when
       projected into the Z dimension */

    /* simplest algorithm, non-AA filled circle, from Apple ML */
    string fc_frag_shader = 
      """
      void main() {
        vec2 dXdY = gl_TexCoord[0].st - 0.5;
        if(dot(dXdY, dXdY) > 0.25)
          discard;
        gl_FragColor = gl_Color;
      }""";

    /* extend the above with anti-aliasing */
    string afc_frag_shader =
      """
      uniform float low;
      uniform float high;
      void main() {
        vec2 dXdY = gl_TexCoord[0].st - 0.5;
        vec2 dXdY2 = dot(dXdY, dXdY);
        float gradient = smoothstep(low, high, dXdY2);
        gl_FragColor = mix(gl_Color, vec4(gl_Color.rgb,0.0), gradient);
      }""";

    /* adapt above for open circles */
    string aoc_frag_shader =
      """
      uniform float halfwidth;
      uniform float r2;
      void main() {
        vec2 dXdY = gl_TexCoord[0].st - 0.5;
        float dXdY2 = dot(dXdY, dXdY);
        float gradient = smoothstep(0.0, halfwidth, abs(r2 - dXdY2));
        gl_FragColor = mix(gl_Color, vec4(gl_Color.rgb,0.0), gradient);
      }""";

    /* like above, but no antialiasing */
    string oc_frag_shader =
      """
      uniform float halfwidth;
      void main() {
        vec2 dXdY = gl_TexCoord[0].st - 0.5;
        if(abs(0.25 - dot(dXdY, dXdY)) > halfwidth)
          discard;
        gl_FragColor = gl_Color;
      }""";

    /* we are not currently using anti-aliasing */
    frag = compile_shader(oc_frag_shader, Cogl.Shader.Type.FRAGMENT_SHADER);
    oc_program = build_program(frag, vert);
    Cogl.Shader.unref(frag);

    frag = compile_shader(fc_frag_shader, Cogl.Shader.Type.FRAGMENT_SHADER);
    fc_program = build_program(frag, vert);
    Cogl.Shader.unref(frag);

    Cogl.Shader.unref(vert);
  }

  private static Cogl.Handle build_program(Cogl.Handle frag, Cogl.Handle vert) {
    Cogl.Handle program = Cogl.Program.create();
    Cogl.Program.attach_shader(program, vert);
    Cogl.Program.attach_shader(program, frag);
    Cogl.Program.link(program);
    return(program);
  }
  
  private static Cogl.Handle compile_shader(string source,
                                            Cogl.Shader.Type type)
  {
    Cogl.Handle shader = Cogl.Shader.create(type);
    Cogl.Shader.source(shader, source);
    Cogl.Shader.compile(shader);

    int code;
    Cogl.Shader.get_parameteriv(shader,
                                Cogl.Shader.Parameter.OBJECT_COMPILE_STATUS,
                                out code);
    if (code != 1) {
      char[] log = new char[1000];
      Cogl.Shader.get_info_log(shader, log);
      critical("shader compile error: %s", (string)log);
    }

    return(shader);
  }
  
  public override void finalize() {
    Cogl.Texture.unref(dummy_texture);
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
    
    //End();
    /*
    /* inspired by: http://slabode.exofire.net/circle_draw.shtml */
    /*
    int num_segments = (int)(2.0f * (float)Math.PI / Math.acosf(1 - 0.25f/r));
    int[] v = new int[2*num_segments];
    float theta = 2 * (float)Math.PI / num_segments; 
    float tangential_factor = Math.tanf(theta);//calculate the tangential factor
    float radial_factor = Math.cosf(theta);//calculate the radial factor 
    float x = r;//we start at angle = 0 
    float y = 0;
    
    for(int i = 0; i < num_segments; i++) {
      v[2*i] = (int)x + cx;
      v[2*i+1] = (int)y + cy;

      //calculate the tangential vector 
      //remember, the radial vector is (x, y) 
      //to get the tangential vector we flip those coordinates and
      // negate one of them 

      float tx = -y; 
      float ty = x; 
        
      //add the tangential vector 

      x += tx * tangential_factor; 
      y += ty * tangential_factor; 
        
      //correct using the radial factor 
      
      x *= radial_factor; 
      y *= radial_factor; 
    }
    */
    //float theta_step = Math.acosf(1 - 0.25f/r);
    //int num_segments = (int)(2*Math.PI/theta_step);

    /*
    float step = (float) Math.PI / (4*r);
    int n = (int)(2/step)*2;
    int[] v = new int[n*4];

    float theta = 0;
    for(int i = 0; i < n; i += 2, theta += step) {
      int xmod = (int)(Math.cosf(theta) * r);
      int ymod = (int)(Math.sinf(theta) * r);
      v[i] = v[4*n-i-2] = cx + xmod;
      v[i+1] = v[2*n-i-1] = cy - ymod;
      v[4*n-i-1] = v[2*n+i+1] = cy + ymod;
      v[2*n-i-2] = v[2*n+i] = cx - xmod;
    }
    */  

    //FIXME: obviously need to setup the transformation matrix here if
    //we are going to use display lists.
    
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
    
    //vertices(Consts.POLYGON, vx, vy);

    /* or use Cogl */
    /* FIXME: filling does not seem to work */
    /*    
    if (fill.alpha > 0) {
      Cogl.Path.ellipse(Clutter.Fixed.from_int(cx), Clutter.Fixed.from_int(cy),
                        Clutter.Fixed.from_int((int)r),
                        Clutter.Fixed.from_int((int)r));
      Cogl.color(to_clutter_color(fill));
      Cogl.Path.fill();
    }
    if (stroke.alpha > 0) {
      Cogl.Path.ellipse(Clutter.Fixed.from_int(cx), Clutter.Fixed.from_int(cy),
                        Clutter.Fixed.from_int((int)r),
                        Clutter.Fixed.from_int((int)r));
      Cogl.color(to_clutter_color(stroke));
      Cogl.Path.stroke();
    }
    */
    /* Notes:
       1) The midpoint algorithm draws the cleanest circles, but they
       are not anti-aliased and filling would require
       modification. Could draw with LINE_LOOP, but that does not work
       well with small circles. Very fast though.
       2) About as fast as midpoint. Circles look.. hand drawn.. though
       3) The naive approach using cosf/sinf is slow, does not work
       well with antialiasing and small circles are malformed.
       4) The Cogl approach is faster, and the circles
       look as good. Could accelerate by drawing only a quarter arc, then
       mirroring. Hmm.. filling just draws a filled
       square... apparently Cogl uses the stencil buffer for filling,
       which may need EXT_packed_depth_stencil to work with FBOs.
    */
    
    /* perhaps use the GPU?
    http://my.opera.com/Vorlath/blog/2008/10/29/gpu-antialiased-circle-drawing
    http://gamedusa.blogspot.com/2007/05/drawing-circle-primitives-using.html
    http://www.lighthouse3d.com/opengl/ledshader/index.php?page2
    http://lists.apple.com/archives/mac-opengl/2007/Aug/msg00150.html
    */

    /* Without anti-aliasing, the shaders are almost as fast as the
       midpoint algorithm for open circles. They look great and are
       actually faster (and about twice as fast as midpoint) when
       filling. But for small circles (r=3), the midpoint is about 3
       times faster for filled circles. The texture drawing overhead
       is high, probably because we don't know what we're doing. */
    
    /*
    int r2_loc = Cogl.Program.get_uniform_location(circle_program, "r2");
    Cogl.Program.uniform_1f(r2_loc, (0.5f-halfwidth)*(0.5f-halfwidth));
    */
    /*
    if (fill.alpha > 0) {
      set_color(fill);
      //  Cogl.Program.use(fc_program);
      draw_dummy_texture(cx, cy, r - 1);
    }
    if (stroke.alpha > 0) {
      set_color(stroke);
      Cogl.Program.use(oc_program);
      int loc = Cogl.Program.get_uniform_location(oc_program, "halfwidth");
      float halfwidth = 1/(4.0f*r);
      Cogl.Program.uniform_1f(loc, halfwidth);
      draw_dummy_texture(cx, cy, r);
    }
    */
    //Cogl.Program.use(Cogl.Handle.INVALID);
    
  }

  private void draw_dummy_texture(int cx, int cy, uint r) {
    Cogl.Texture.rectangle(dummy_texture,
                           Clutter.Fixed.from_int((int)(cx - r)),
                           Clutter.Fixed.from_int((int)(cy - r)),
                           Clutter.Fixed.from_int((int)(cx + r)),
                           Clutter.Fixed.from_int((int)(cy + r)),
                           Clutter.Fixed.from_int(0),
                           Clutter.Fixed.from_int(0),
                           Clutter.Fixed.from_float(1),
                           Clutter.Fixed.from_float(1));
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
    int i = 0, j = 0;

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
      /*
      v[4*tx+4] = cx; v[4*tx+5] = cy - (int)r;
      v[2*tx+2] = cx + (int)r; v[2*tx+3] = cy;
      v[6*tx+6] = cx - (int)r; v[6*tx+7] = cy;
      */
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
    
    /* Filled midpoint */
    /* Draw lines when 'y' changes from:
       8*tx-i+6,7 to i+2,3
       4*tx+i+6,7 to 4*tx-i+2,3
       6*tx+i+8,9 to 2*tx-i+0,1
       6*tx-i+4,5 to 2*tx+i+4,5
       Also from 6*tx+6,7 to 2*tx+2,3
    */
    
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
      /*
      if (fill) {
        v[j++] = cx - y; v[j++] = cy + x;
        v[j++] = cx + y; v[j++] = cy + x;
        v[j++] = cx - y; v[j++] = cy - x;
        v[j++] = cx + y; v[j++] = cy - x;
      } else {
        v[i+2] = cx + x; v[i+3] = cy + y;
        v[8*tx-i+6] = cx - x; v[8*tx-i+7] = cy + y;
        v[4*tx-i+2] = cx + x; v[4*tx-i+3] = cy - y;
        v[4*tx+i+6] = cx - x; v[4*tx+i+7] = cy - y;
        v[2*tx-i] = cx + y; v[2*tx-i+1] = cy + x;
        v[6*tx+i+8] = cx - y; v[6*tx+i+9] = cy + x;
        v[2*tx+i+4] = cx + y; v[2*tx+i+5] = cy - x;
        v[6*tx-i+4] = cx - y; v[6*tx-i+5] = cy - x;
        i+=2;
      }
      */
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