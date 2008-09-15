using Clutter;

[Import ()]
[CCode (cprefix = "COGL", lower_case_cprefix = "cogl_", cheader_filename = "cogl/cogl.h")]
namespace Cogl {
        [CCode (cprefix = "COGL_PIXEL_FORMAT_", has_type_id = false)]
        public enum PixelFormat {
                ANY,
                A_8,
                RGB_888,
                BGR_888,
                RGBA_8888,
                BGRA_8888,
                ARGB_8888,
                ABGR_8888,
                RGBA_8888_PRE,
                BGRA_8888_PRE,
                ARGB_8888_PRE,
                ABGR_8888_PRE,
                RGB_565,
                RGBA_4444,
                RGBA_5551,
                RGBA_4444_PRE,
                RGBA_5551_PRE,
                YUV,
                G_8
        }

        [CCode (cprefix = "COGL_FEATURE_", has_type_id = false)]
        public enum FeatureFlags {
                TEXTURE_RECTANGLE,
                TEXTURE_NPOT,
                TEXTURE_YUV,
                TEXTURE_READ_PIXELS,
                SHADERS_GLSL,
                OFFSCREEN,
                OFFSCREEN_MULTISAMPLE,
                OFFSCREEN_BLIT,
                FOUR_CLIP_PLANES,
                STENCIL_BUFFER
        }

        [Flags]
        [CCode (cprefix = "COGL_", has_type_id = false)]
        public enum BufferTarget {
                WINDOW_BUFFER,
                MASK_BUFFER,
                OFFSCREEN_BUFFER
        }

        public struct TextureVertex {
                public Clutter.Fixed x;
                public Clutter.Fixed y;
                public Clutter.Fixed z;
                public Clutter.Fixed tx;
                public Clutter.Fixed ty;
                public Clutter.Color color;
        }

        public static delegate void FuncPtr ();

        public static FuncPtr get_proc_address (string name);

        public static bool check_extension (string name, string ext);

        public static void get_bitmasks (out int red, out int green, out int blue, out int alpha);

        public static void perspective (Clutter.Fixed fovy, Clutter.Fixed aspect, Clutter.Fixed z_near, Clutter.Fixed z_far);

        public static void setup_viewport (uint width, uint height, Clutter.Fixed fovy, Clutter.Fixed aspect, Clutter.Fixed z_near, Clutter.Fixed z_far);

        namespace Matrix {
                [CCode (cname = "cogl_push_matrix")]
                public static void push ();
                [CCode (cname = "cogl_pop_matrix")]
                public static void pop ();
        }

        namespace Transform {
                [CCode (cname = "cogl_scale")]
                public static void scale (Clutter.Fixed x, Clutter.Fixed y);

                [CCode (cname = "cogl_translatex")]
                public static void translate (Clutter.Fixed x, Clutter.Fixed y, Clutter.Fixed z);

                [CCode (cname = "cogl_rotatex")]
                public static void rotate (Clutter.Fixed angle, int x, int y, int z);
        }

        namespace Clip {
                [CCode (cname = "cogl_clip_set")]
                public static void set (Clutter.Fixed x_offset, Clutter.Fixed y_offset, Clutter.Fixed width, Clutter.Fixed height);
                [CCode (cname = "cogl_clip_unset")]
                public static void unset ();
        }

        public static void enable_depth_test ();

        public static void alpha_func (int compare_func, Clutter.Fixed reference_value);

        public static void fog_set (Clutter.Color fog_color, Clutter.Fixed density, Clutter.Fixed z_near, Clutter.Fixed z_far);

        public static void paint_init (Clutter.Color color);

        public static void rectangle (int x, int y, uint width, uint height);

        public static void color (Clutter.Color color);

        namespace Path {
                [CCode (cname = "cogl_path_fill")]
                public static void fill ();

                [CCode (cname = "cogl_path_stroke")]
                public static void stroke ();

                [CCode (cname = "cogl_path_move_to")]
                public static void move_to (Clutter.Fixed x, Clutter.Fixed y);
                [CCode (cname = "cogl_path_rel_move_to")]
                public static void rel_move_to (Clutter.Fixed x, Clutter.Fixed y);

                [CCode (cname = "cogl_path_line_to")]
                public static void line_to (Clutter.Fixed x, Clutter.Fixed y);
                [CCode (cname = "cogl_path_rel_line_to")]
                public static void rel_line_to (Clutter.Fixed x, Clutter.Fixed y);

                [CCode (cname = "cogl_path_arc")]
                public static void arc (Clutter.Fixed center_x, Clutter.Fixed center_y, Clutter.Fixed radius_x, Clutter.Fixed radius_y, Clutter.Angle angle_1, Clutter.Angle angle_2);

                [CCode (cname = "cogl_path_curve_to")]
                public static void curve_to (Clutter.Fixed x1, Clutter.Fixed y1, Clutter.Fixed x2, Clutter.Fixed y2, Clutter.Fixed x3, Clutter.Fixed y3);
                public static void rel_curve_to (Clutter.Fixed x1, Clutter.Fixed y1, Clutter.Fixed x2, Clutter.Fixed y2, Clutter.Fixed x3, Clutter.Fixed y3);

                [CCode (cname = "cogl_path_close")]
                public static void close ();

                [CCode (cname = "cogl_path_line")]
                public static void line (Clutter.Fixed x1, Clutter.Fixed y1, Clutter.Fixed x2, Clutter.Fixed y2);

                [NoArrayLength]
                [CCode (cname = "cogl_path_polyline")]
                public static void polyline (Clutter.Fixed[] coords, int n_points);

                [NoArrayLength]
                [CCode (cname = "cogl_path_polygon")]
                public static void polygon (Clutter.Fixed[] coords, int n_points);

                [CCode (cname = "cogl_path_rectangle")]
                public static void rectangle (Clutter.Fixed x, Clutter.Fixed y, Clutter.Fixed width, Clutter.Fixed height);

                [CCode (cname = "cogl_path_ellipse")]
                public static void ellipse (Clutter.Fixed center_x, Clutter.Fixed center_y, Clutter.Fixed radius_x, Clutter.Fixed radius_y);

                [CCode (cname = "cogl_path_round_rectangle")]
                public static void round_rectangle (Clutter.Fixed x, Clutter.Fixed y, Clutter.Fixed width, Clutter.Fixed height, Clutter.Fixed radius, Clutter.Angle arc_step);
        }

        [SimpleType]
        [CCode (cname = "CoglHandle", has_type_id = "false", default_value = "COGL_INVALID_HANDLE")]
        public struct Handle {
                [CCode (cname = "cogl_is_texture")]
                public bool is_texture ();
                [CCode (cname = "cogl_is_shader")]
                public bool is_shader ();
                [CCode (cname = "cogl_is_program")]
                public bool is_program ();
                [CCode (cname = "cogl_is_offscreen")]
                public bool is_offscreen ();
                
                [CCode (cname = "COGL_INVALID_HANDLE")]
                public const Handle INVALID;
        }

        namespace Texture {
                public static Handle new_from_file (string filename, int max_waste, bool auto_mipmap, PixelFormat internal_format) throws GLib.Error;
                public static Handle new_with_size (uint width, uint height, int max_waste, bool auto_mipmap, PixelFormat internal_format);
                public static void rectangle (Handle handle, Clutter.Fixed x1, Clutter.Fixed y1, Clutter.Fixed x2, Clutter.Fixed y2, Clutter.Fixed tx1, Clutter.Fixed ty1, Clutter.Fixed tx2, Clutter.Fixed ty2);

                [NoArrayLength]
                public static void polygon (Handle handle, uint n_vertices, TextureVertex[] vertices, bool use_color);

                public void unref(Handle handle);
        }

        namespace Offscreen {
                public static Handle new_to_texture (Handle texhandle);
                public void unref(Handle handle);
        }

        public static void draw_buffer (BufferTarget target, Handle offscreen);
}
