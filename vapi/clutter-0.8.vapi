/* clutter-0.8.vapi
 *
 * Copyright (C) 2007  Alberto Ruiz
 * Copyright (C) 2007  OpenedHand Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

[CCode (cprefix = "Clutter", lower_case_cprefix = "clutter_")]
namespace Clutter {
	[CCode (cprefix = "CLUTTER_", cheader_filename = "clutter/clutter.h")]
	public enum EventType {
		NOTHING,
		KEY_PRESS,
		KEY_RELEASE,
		MOTION,
		ENTER,
		LEAVE,
		BUTTON_PRESS,
		BUTTON_RELEASE,
		SCROLL,
		STAGE_STATE,
		DESTROY_NOTIFY,
		CLIENT_MESSAGE,
		DELETE
	}

	[CCode (cprefix = "CLUTTER_GRAVITY_", cheader_filename = "clutter/clutter.h")]
	public enum Gravity {
		NONE,
		NORTH,
		NORTH_EAST,
		EAST,
		SOUTH_EAST,
		SOUTH,
		SOUTH_WEST,
		WEST,
		NORTH_WEST,
		CENTER
	}

	[CCode (cprefix = "CLUTTER_INIT_", cheader_filename = "clutter/clutter.h")]
	public enum InitError {
		SUCCESS,
		ERROR_UNKNOWN,
		ERROR_THREADS,
		ERROR_BACKEND,
		ERROR_INTERNAL
	}

	[CCode (cprefix = "CLUTTER_REQUEST_", cheader_filename = "clutter/clutter.h")]
	public enum RequestMode {
		HEIGHT_FOR_WIDTH,
		WIDTH_FOR_HEIGHT
	}

	[CCode (cprefix = "CLUTTER_", cheader_filename = "clutter/clutter.h")]
	public enum RotateAxis {
		X_AXIS,
		Y_AXIS,
		Z_AXIS
	}

	[CCode (cprefix = "CLUTTER_ROTATE_", cheader_filename = "clutter/clutter.h")]
	public enum RotateDirection {
		CW,
		CCW
	}

	[CCode (cprefix = "CLUTTER_SCRIPT_ERROR_", cheader_filename = "clutter/clutter.h")]
	public errordomain ScriptError {
		INVALID_TYPE_FUNCTION,
		INVALID_PROPERTY,
		INVALID_VALUE
	}

	[CCode (cprefix = "CLUTTER_SCROLL_", cheader_filename = "clutter/clutter.h")]
	public enum ScrollDirection {
		UP,
		DOWN,
		LEFT,
		RIGHT
	}

	[CCode (cprefix = "CLUTTER_TEXTURE_QUALITY_", cheader_filename = "clutter/clutter.h")]
	public enum TextureQuality {
		LOW,
		MEDIUM,
		HIGH
	}

	[CCode (cprefix = "CLUTTER_TIMELINE_", cheader_filename = "clutter/clutter.h")]
	public enum TimelineDirection {
		FORWARD,
		BACKWARD
	}

	[CCode (cprefix = "CLUTTER_ACTOR_", cheader_filename = "clutter/clutter.h")]
	[Flags]
	public enum ActorFlags {
		MAPPED,
		REALIZED,
		REACTIVE
	}

	[CCode (cprefix = "CLUTTER_EVENT_FLAG_", cheader_filename = "clutter/clutter.h")]
	[Flags]
	public enum EventFlags {
		SYNTHETIC
	}

	[CCode (cprefix = "CLUTTER_FEATURE_", cheader_filename = "clutter/clutter.h")]
	[Flags]
	public enum FeatureFlags {
		TEXTURE_NPOT,
		SYNC_TO_VBLANK,
		TEXTURE_YUV,
		TEXTURE_READ_PIXELS,
		STAGE_STATIC,
		STAGE_USER_RESIZE,
		STAGE_CURSOR,
		SHADERS_GLSL,
		OFFSCREEN,
		STAGE_MULTIPLE
	}

	[CCode (cprefix = "CLUTTER_", cheader_filename = "clutter/clutter.h")]
	[Flags]
	public enum ModifierType {
		SHIFT_MASK,
		LOCK_MASK,
		CONTROL_MASK,
		MOD1_MASK,
		MOD2_MASK,
		MOD3_MASK,
		MOD4_MASK,
		MOD5_MASK,
		BUTTON1_MASK,
		BUTTON2_MASK,
		BUTTON3_MASK,
		BUTTON4_MASK,
		BUTTON5_MASK
	}

	[CCode (cprefix = "CLUTTER_STAGE_STATE_", cheader_filename = "clutter/clutter.h")]
	[Flags]
	public enum StageState {
		FULLSCREEN,
		OFFSCREEN,
		ACTIVATED
	}

	[CCode (cprefix = "CLUTTER_TEXTURE_", cheader_filename = "clutter/clutter.h")]
	[Flags]
	public enum TextureFlags {
		RGB_FLAG_BGR,
		RGB_FLAG_PREMULT,
		YUV_FLAG_YUV2
	}

	[CCode (cprefix = "CLUTTER_SHADER_ERROR_", cheader_filename = "clutter/clutter.h")]
	public errordomain ShaderError {
		NO_ASM,
		NO_GLSL,
		COMPILE,
	}

	[CCode (cprefix = "CLUTTER_TEXTURE_ERROR_", cheader_filename = "clutter/clutter.h")]
	public errordomain TextureError {
		OUT_OF_MEMORY,
		NO_YUV,
		BAD_FORMAT,
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public abstract class Actor : GLib.InitiallyUnowned, Clutter.Scriptable {
		public uint flags;
		[CCode (cname = "CLUTTER_ACTOR_FLAGS")]
		public Clutter.ActorFlags get_flags ();
		[CCode (cname = "CLUTTER_ACTOR_SET_FLAGS")]
		public void set_flags (Clutter.ActorFlags flags);
		[CCode (cname = "CLUTTER_ACTOR_UNSET_FLAGS")]
		public void unset_flags (Clutter.ActorFlags flags);
		public void allocate_preferred_size (bool absolute_origin_changed);
		public void apply_relative_transform_to_point (Clutter.Actor ancestor, Clutter.Vertex point, Clutter.Vertex vertex);
		public void apply_transform_to_point (Clutter.Vertex point, Clutter.Vertex vertex);
		public void get_abs_allocation_vertices (Clutter.Vertex[] verts);
		public void get_allocation_box (out Clutter.ActorBox box);
		public void get_allocation_coords (out int x_1, out int y_1, out int x_2, out int y_2);
		public void get_allocation_geometry (Clutter.Geometry geom);
		public void get_allocation_vertices (Clutter.Actor ancestor, Clutter.Vertex[] verts);
		public void get_anchor_point (out int anchor_x, out int anchor_y);
		public void get_anchor_pointu (out Clutter.Unit anchor_x, out Clutter.Unit anchor_y);
		public void get_clip (out int xoff, out int yoff, out int width, out int height);
		public void get_clipu (out Clutter.Unit xoff, out Clutter.Unit yoff, out Clutter.Unit width, out Clutter.Unit height);
		public int get_depth ();
		public Clutter.Unit get_depthu ();
		public bool get_fixed_position_set ();
		public void get_geometry (out Clutter.Geometry geometry);
		public uint get_gid ();
		public uint get_height ();
		public Clutter.Unit get_heightu ();
		public weak string get_name ();
		public uchar get_opacity ();
		public uchar get_paint_opacity ();
		public weak Clutter.Actor get_parent ();
		public void get_position (out int x, out int y);
		public void get_positionu (out Clutter.Unit x, out Clutter.Unit y);
		public void get_preferred_size (out Clutter.Unit min_width_p, out Clutter.Unit min_height_p, out Clutter.Unit natural_width_p, out Clutter.Unit natural_height_p);
		public bool get_reactive ();
		public double get_rotation (Clutter.RotateAxis axis, out int x, out int y, out int z);
		public double get_rotationu (Clutter.RotateAxis axis, out Clutter.Unit x, out Clutter.Unit y, out Clutter.Unit z);
		public Clutter.Fixed get_rotationx (Clutter.RotateAxis axis, int x, int y, int z);
		public void get_scale (out double scale_x, out double scale_y);
		public void get_scalex (out Clutter.Fixed scale_x, out Clutter.Fixed scale_y);
		public weak Clutter.Shader get_shader ();
		public void get_size (out uint width, out uint height);
		public void get_sizeu (out Clutter.Unit width, out Clutter.Unit height);
		public weak Clutter.Actor get_stage ();
		public void get_transformed_position (out int x, out int y);
		public void get_transformed_positionu (out Clutter.Unit x, out Clutter.Unit y);
		public void get_transformed_size (out uint width, out uint height);
		public void get_transformed_sizeu (out Clutter.Unit width, out Clutter.Unit height);
		public uint get_width ();
		public Clutter.Unit get_widthu ();
		public int get_x ();
		public Clutter.Unit get_xu ();
		public int get_y ();
		public Clutter.Unit get_yu ();
		[CCode (cname = "clutter_actor_has_clip")]
		public bool get_has_clip ();
		public bool is_rotated ();
		public bool is_scaled ();
		public void lower (Clutter.Actor above);
		public void lower_bottom ();
		public void move_anchor_point (int anchor_x, int anchor_y);
		public void move_anchor_point_from_gravity (Clutter.Gravity gravity);
		public void move_anchor_pointu (Clutter.Unit anchor_x, Clutter.Unit anchor_y);
		public void move_by (int dx, int dy);
		public void move_byu (Clutter.Unit dx, Clutter.Unit dy);
		public void queue_redraw ();
		public void queue_relayout ();
		public void raise (Clutter.Actor below);
		public void raise_top ();
		public void remove_clip ();
		public void reparent (Clutter.Actor new_parent);
		public void set_anchor_point (int anchor_x, int anchor_y);
		public void set_anchor_point_from_gravity (Clutter.Gravity gravity);
		public void set_anchor_pointu (Clutter.Unit anchor_x, Clutter.Unit anchor_y);
		public void set_clip (int xoff, int yoff, int width, int height);
		public void set_clipu (Clutter.Unit xoff, Clutter.Unit yoff, Clutter.Unit width, Clutter.Unit height);
		public void set_depth (int depth);
		public void set_depthu (Clutter.Unit depth);
		public void set_fixed_position_set (bool is_set);
		public void set_geometry (Clutter.Geometry geometry);
		public void set_height (uint height);
		public void set_heightu (Clutter.Unit height);
		public void set_name (string name);
		public void set_opacity (uchar opacity);
		public void set_parent (Clutter.Actor parent);
		public void set_position (int x, int y);
		public void set_positionu (Clutter.Unit x, Clutter.Unit y);
		public void set_reactive (bool reactive);
		public void set_rotation (Clutter.RotateAxis axis, double angle, int x, int y, int z);
		public void set_rotationu (Clutter.RotateAxis axis, double angle, Clutter.Unit x, Clutter.Unit y, Clutter.Unit z);
		public void set_rotationx (Clutter.RotateAxis axis, Clutter.Fixed angle, int x, int y, int z);
		public void set_scale (double scale_x, double scale_y);
		public void set_scalex (Clutter.Fixed scale_x, Clutter.Fixed scale_y);
		public bool set_shader (Clutter.Shader shader);
		public void set_shader_param (string param, float value);
		public void set_size (int width, int height);
		public void set_sizeu (Clutter.Unit width, Clutter.Unit height);
		public void set_width (uint width);
		public void set_widthu (Clutter.Unit width);
		public void set_x (int x);
		public void set_xu (Clutter.Unit x);
		public void set_y (int y);
		public void set_yu (Clutter.Unit y);
		public bool should_pick_paint ();
		public bool transform_stage_point (Clutter.Unit x, Clutter.Unit y, out Clutter.Unit x_out, out Clutter.Unit y_out);
		public void unparent ();
		public virtual void allocate (Clutter.ActorBox box, bool absolute_origin_changed);
		public virtual void get_preferred_height (Clutter.Unit for_width, out Clutter.Unit min_height_p, out Clutter.Unit natural_height_p);
		public virtual void get_preferred_width (Clutter.Unit for_height, out Clutter.Unit min_width_p, out Clutter.Unit natural_width_p);
		public virtual void hide_all ();
		public virtual void pick (Clutter.Color color);
		public virtual void show_all ();
		[NoAccessorMethod]
		public Clutter.ActorBox allocation { get; }
		[NoAccessorMethod]
		public int anchor_x { get; set; }
		[NoAccessorMethod]
		public int anchor_y { get; set; }
		public Clutter.Geometry clip { get; set; }
		public int depth { get; set; }
		public bool fixed_position_set { get; set; }
		[NoAccessorMethod]
		public Clutter.Unit fixed_x { get; set; }
		[NoAccessorMethod]
		public Clutter.Unit fixed_y { get; set; }
		[NoAccessorMethod]
		public bool has_clip { get; }
		public int height { get; set; }
		[NoAccessorMethod]
		public Clutter.Unit min_height { get; set; }
		[NoAccessorMethod]
		public bool min_height_set { get; set; }
		[NoAccessorMethod]
		public Clutter.Unit min_width { get; set; }
		[NoAccessorMethod]
		public bool min_width_set { get; set; }
		public string name { get; set; }
		[NoAccessorMethod]
		public Clutter.Unit natural_height { get; set; }
		[NoAccessorMethod]
		public bool natural_height_set { get; set; }
		[NoAccessorMethod]
		public Clutter.Unit natural_width { get; set; }
		[NoAccessorMethod]
		public bool natural_width_set { get; set; }
		public uchar opacity { get; set; }
		public bool reactive { get; set; }
		[NoAccessorMethod]
		public Clutter.RequestMode request_mode { get; set; }
		[NoAccessorMethod]
		public double rotation_angle_x { get; set; }
		[NoAccessorMethod]
		public double rotation_angle_y { get; set; }
		[NoAccessorMethod]
		public double rotation_angle_z { get; set; }
		[NoAccessorMethod]
		public Clutter.Vertex rotation_center_x { get; set; }
		[NoAccessorMethod]
		public Clutter.Vertex rotation_center_y { get; set; }
		[NoAccessorMethod]
		public Clutter.Vertex rotation_center_z { get; set; }
		[NoAccessorMethod]
		public double scale_x { get; set; }
		[NoAccessorMethod]
		public double scale_y { get; set; }
		[NoAccessorMethod]
		public bool show_on_set_parent { get; set; }
		[NoAccessorMethod]
		public bool visible { get; set; }
		public int width { get; set; }
		public int x { get; set; }
		public int y { get; set; }
		public virtual signal bool button_press_event (Clutter.ButtonEvent event);
		public virtual signal bool button_release_event (Clutter.ButtonEvent event);
		public virtual signal bool captured_event (Clutter.Event event);
		[HasEmitter]
		public virtual signal void destroy ();
		public virtual signal bool enter_event (Clutter.CrossingEvent event);
		[HasEmitter]
		public virtual signal bool event (Clutter.Event event);
		public virtual signal void focus_in ();
		public virtual signal void focus_out ();
		[HasEmitter]
		public virtual signal void hide ();
		public virtual signal bool key_press_event (Clutter.KeyEvent event);
		public virtual signal bool key_release_event (Clutter.KeyEvent event);
		public virtual signal bool leave_event (Clutter.CrossingEvent event);
		public virtual signal bool motion_event (Clutter.MotionEvent event);
		[HasEmitter]
		public virtual signal void paint ();
		public virtual signal void parent_set (Clutter.Actor? old_parent);
		[HasEmitter]
		public virtual signal void realize ();
		public virtual signal bool scroll_event (Clutter.ScrollEvent event);
		[HasEmitter]
		public virtual signal void show ();
		[HasEmitter]
		public virtual signal void unrealize ();
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class BehaviourPath : Clutter.Behaviour, Clutter.Scriptable {
		public BehaviourPath (Clutter.Alpha alpha, Clutter.Knot[] knots, uint n_knots);
		public void append_knot (ref Clutter.Knot knot);
		public void append_knots (...);
		public void clear ();
		public weak GLib.SList get_knots ();
		public void insert_knot (uint offset, Clutter.Knot knot);
		public void remove_knot (uint offset);
		[NoAccessorMethod]
		public Clutter.Knot knot { set; }
		public virtual signal void knot_reached (Clutter.Knot knot);
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class BehaviourBspline : Clutter.Behaviour, Clutter.Scriptable {
		public BehaviourBspline (Clutter.Alpha alpha, Clutter.Knot[] knots, uint n_knots);
		public void adjust (uint offset, Clutter.Knot knot);
		public void append_knot (ref Clutter.Knot knot);
		public void append_knots (...);
		public void clear ();
		public void get_origin (out Clutter.Knot knot);
		public void join (Clutter.BehaviourBspline bs2);
		public void set_origin (Clutter.Knot knot);
		public weak Clutter.Behaviour split (uint offset);
		public void truncate (uint offset);
		public virtual signal void knot_reached (Clutter.Knot knot);
	}

	[Compact]
	[CCode (cheader_filename = "clutter/clutter.h")]
	public class AnyEvent {
		public Clutter.EventType type;
		public uint time;
		public Clutter.EventFlags flags;
		public weak Clutter.Stage stage;
		public weak Clutter.Actor source;
	}

	[Compact]
	[CCode (cheader_filename = "clutter/clutter.h")]
	public class ButtonEvent {
		public Clutter.EventType type;
		public uint time;
		public Clutter.EventFlags flags;
		public weak Clutter.Stage stage;
		public weak Clutter.Actor source;
		public int x;
		public int y;
		public Clutter.ModifierType modifier_state;
		public uint button;
		public uint click_count;
		public double axes;
		public weak Clutter.InputDevice device;
		[CCode (cname = "clutter_button_event_button")]
		public uint get_button ();
	}

	[Compact]
	[CCode (cheader_filename = "clutter/clutter.h")]
	public class CrossingEvent {
		public Clutter.EventType type;
		public uint time;
		public Clutter.EventFlags flags;
		public weak Clutter.Stage stage;
		public weak Clutter.Actor source;
		public int x;
		public int y;
		public weak Clutter.InputDevice device;
		public weak Clutter.Actor related;
	}

	[Compact]
	[CCode (cheader_filename = "clutter/clutter.h")]
	public class InputDevice {
	}

	[Compact]
	[CCode (cheader_filename = "clutter/clutter.h")]
	public class KeyEvent {
		public Clutter.EventType type;
		public uint time;
		public Clutter.EventFlags flags;
		public weak Clutter.Stage stage;
		public weak Clutter.Actor source;
		public Clutter.ModifierType modifier_state;
		public uint keyval;
		public ushort hardware_keycode;
		public unichar unicode_value;
		[CCode (cname = "clutter_key_event_code")]
		public ushort get_code ();
		[CCode (cname = "clutter_key_event_symbol")]
		public uint get_symbol ();
		[CCode (cname = "clutter_key_event_unicode")]
		public uint get_unicode ();
	}

	[Compact]
	[CCode (cheader_filename = "clutter/clutter.h")]
	public class MotionEvent {
		public Clutter.EventType type;
		public uint time;
		public Clutter.EventFlags flags;
		public weak Clutter.Stage stage;
		public weak Clutter.Actor source;
		public int x;
		public int y;
		public Clutter.ModifierType modifier_state;
		public double axes;
		public weak Clutter.InputDevice device;
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class ParamSpecFixed : GLib.ParamSpec {
		public Clutter.Fixed minimum;
		public Clutter.Fixed maximum;
		public Clutter.Fixed default_value;

                [CCode (cname = "clutter_param_spec_fixed")]
                public ParamSpecFixed (string name, string nick, string blurb, Fixed minimum, Fixed maximum, Fixed default_value, GLib.ParamFlags flags);
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class ParamSpecUnit : GLib.ParamSpec {
		public Clutter.Unit minimum;
		public Clutter.Unit maximum;
		public Clutter.Unit default_value;

                [CCode (cname = "clutter_param_spec_unit")]
                public ParamSpecUnit (string name, string nick, string blurb, Unit minimum, Unit maximum, Unit default_value, GLib.ParamFlags flags);
	}

	[Compact]
	[CCode (cheader_filename = "clutter/clutter.h")]
	public class ScrollEvent {
		public Clutter.EventType type;
		public uint time;
		public Clutter.EventFlags flags;
		public weak Clutter.Stage stage;
		public weak Clutter.Actor source;
		public int x;
		public int y;
		public Clutter.ScrollDirection direction;
		public Clutter.ModifierType modifier_state;
		public double axes;
		public weak Clutter.InputDevice device;
	}

	[Compact]
	[CCode (cheader_filename = "clutter/clutter.h")]
	public class StageStateEvent {
		public Clutter.EventType type;
		public uint time;
		public Clutter.EventFlags flags;
		public weak Clutter.Stage stage;
		public weak Clutter.Actor source;
		public Clutter.StageState changed_mask;
		public Clutter.StageState new_state;
	}

	[Compact]
	[CCode (cheader_filename = "clutter/clutter.h")]
	public class TimeoutPool {
		public uint add (uint interval, GLib.SourceFunc func);
		public TimeoutPool (int priority);
		public void remove (uint id);
	}

	[Compact]
	[CCode (copy_function = "clutter_event_copy", cheader_filename = "clutter/clutter.h")]
	public class Event {
		public Clutter.EventType type;
		public weak Clutter.AnyEvent any;
		public weak Clutter.ButtonEvent button;
		public weak Clutter.KeyEvent key;
		public weak Clutter.MotionEvent motion;
		public weak Clutter.ScrollEvent scroll;
		public weak Clutter.StageStateEvent stage_state;
		public weak Clutter.CrossingEvent crossing;
		public weak Clutter.Event copy ();
		public static weak Clutter.Event get ();
		public void get_coords (out int x, out int y);
		public int get_device_id ();
		public weak Clutter.Actor get_source ();
		public weak Clutter.Stage get_stage ();
		public Clutter.ModifierType get_state ();
		public uint get_time ();
		public Event (Clutter.EventType type);
		public static weak Clutter.Event peek ();
		public void put ();
		[CCode (cname = "clutter_event_type")]
		public Clutter.EventType get_type ();
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public struct Geometry {
		public int x;
		public int y;
		public uint width;
		public uint height;
	}

	[Compact]
	[CCode (cheader_filename = "clutter/clutter.h")]
	public class TextureHandle {
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class Alpha : GLib.InitiallyUnowned {
		public uint get_alpha ();
		public weak Clutter.Timeline get_timeline ();
		public Alpha ();
		public Alpha.full (Clutter.Timeline timeline, Clutter.AlphaFunc func, void* data, GLib.DestroyNotify? destroy);
		public void set_closure (GLib.Closure closure);
		public void set_func (Clutter.AlphaFunc func, void* data, GLib.DestroyNotify destroy);
		public void set_timeline (Clutter.Timeline timeline);
		public uint alpha { get; }
		public Clutter.Timeline timeline { get; set; }
                [CCode (cname = "CLUTTER_ALPHA_MAX_ALPHA")]
        	public const int MAX_ALPHA;
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public abstract class Backend : GLib.Object {
                [CCode (cname = "clutter_get_default_backend")]
                public static weak Clutter.Backend get_default ();
		public uint get_double_click_distance ();
		public uint get_double_click_time ();
		public weak Cairo.FontOptions get_font_options ();
		public double get_resolution ();
		public void set_double_click_distance (uint distance);
		public void set_double_click_time (uint msec);
		public void set_font_options (Cairo.FontOptions options);
		public void set_resolution (double dpi);
		[NoWrapper]
		public virtual void add_options (GLib.OptionGroup group);
		[NoWrapper]
		public virtual weak Clutter.Actor create_stage (Clutter.Stage wrapper) throws GLib.Error;
		[NoWrapper]
		public virtual void ensure_context (Clutter.Stage stage);
		[NoWrapper]
		public virtual Clutter.FeatureFlags get_features ();
		[NoWrapper]
		public virtual void init_events ();
		[NoWrapper]
		public virtual void init_features ();
		[NoWrapper]
		public virtual bool post_parse () throws GLib.Error;
		[NoWrapper]
		public virtual bool pre_parse () throws GLib.Error;
		[NoWrapper]
		public virtual void redraw (Clutter.Stage stage);
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public abstract class Behaviour : GLib.Object {
		public void actors_foreach (Clutter.BehaviourForeachFunc func, void* data);
		public void apply (Clutter.Actor actor);
		public weak GLib.SList get_actors ();
		public weak Clutter.Alpha get_alpha ();
		public int get_n_actors ();
		public weak Clutter.Actor get_nth_actor (int index_);
		public bool is_applied (Clutter.Actor actor);
		public void remove_all ();
		public void set_alpha (Clutter.Alpha alpha);
		[NoWrapper]
		public virtual void alpha_notify (uint alpha_value);
		public Clutter.Alpha alpha { get; set; }
		public virtual signal void applied (Clutter.Actor actor);
		public virtual signal void removed (Clutter.Actor actor);
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class BehaviourDepth : Clutter.Behaviour {
		public void get_bounds (out int depth_start, out int depth_end);
		public BehaviourDepth (Clutter.Alpha alpha, int depth_start, int depth_end);
		public void set_bounds (int depth_start, int depth_end);
		[NoAccessorMethod]
		public int depth_end { get; set; }
		[NoAccessorMethod]
		public int depth_start { get; set; }
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class BehaviourEllipse : Clutter.Behaviour {
		public double get_angle_end ();
		public Clutter.Fixed get_angle_endx ();
		public double get_angle_start ();
		public Clutter.Fixed get_angle_startx ();
		public double get_angle_tilt (Clutter.RotateAxis axis);
		public Clutter.Fixed get_angle_tiltx (Clutter.RotateAxis axis);
		public void get_center (out int x, out int y);
		public Clutter.RotateDirection get_direction ();
		public int get_height ();
		public void get_tilt (out double angle_tilt_x, out double angle_tilt_y, out double angle_tilt_z);
		public void get_tiltx (out Clutter.Fixed angle_tilt_x, out Clutter.Fixed angle_tilt_y, out Clutter.Fixed angle_tilt_z);
		public int get_width ();
		public BehaviourEllipse (Clutter.Alpha alpha, int x, int y, int width, int height, Clutter.RotateDirection direction, double start, double end);
		[CCode (cname = "clutter_behaviour_ellipse_newx")]
		public BehaviourEllipse.newx (Clutter.Alpha alpha, int x, int y, int width, int height, Clutter.RotateDirection direction, Clutter.Fixed start, Clutter.Fixed end);
		public void set_angle_end (double angle_end);
		public void set_angle_endx (Clutter.Fixed angle_end);
		public void set_angle_start (double angle_start);
		public void set_angle_startx (Clutter.Fixed angle_start);
		public void set_angle_tilt (Clutter.RotateAxis axis, double angle_tilt);
		public void set_angle_tiltx (Clutter.RotateAxis axis, Clutter.Fixed angle_tilt);
		public void set_center (int x, int y);
		public void set_direction (Clutter.RotateDirection direction);
		public void set_height (int height);
		public void set_tilt (double angle_tilt_x, double angle_tilt_y, double angle_tilt_z);
		public void set_tiltx (Clutter.Fixed angle_tilt_x, Clutter.Fixed angle_tilt_y, Clutter.Fixed angle_tilt_z);
		public void set_width (int width);
		[NoWrapper]
		public virtual void knot_reached (Clutter.Knot knot);
		public double angle_end { get; set; }
		public double angle_start { get; set; }
		[NoAccessorMethod]
		public double angle_tilt_x { get; set; }
		[NoAccessorMethod]
		public double angle_tilt_y { get; set; }
		[NoAccessorMethod]
		public double angle_tilt_z { get; set; }
		public Clutter.Knot center { get; set; }
		public Clutter.RotateDirection direction { get; set; }
		public int height { get; set; }
		public int width { get; set; }
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class BehaviourOpacity : Clutter.Behaviour {
		public void get_bounds (out uchar opacity_start, out uchar opacity_end);
		public BehaviourOpacity (Clutter.Alpha alpha, uchar opacity_start, uchar opacity_end);
		public void set_bounds (uchar opacity_start, uchar opacity_end);
		[NoAccessorMethod]
		public uint opacity_end { get; set; }
		[NoAccessorMethod]
		public uint opacity_start { get; set; }
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class BehaviourRotate : Clutter.Behaviour {
		public Clutter.RotateAxis get_axis ();
		public void get_bounds (out double angle_start, out double angle_end);
		public void get_boundsx (out Clutter.Fixed angle_start, out Clutter.Fixed angle_end);
		public void get_center (int x, int y, int z);
		public Clutter.RotateDirection get_direction ();
		public BehaviourRotate (Clutter.Alpha alpha, Clutter.RotateAxis axis, Clutter.RotateDirection direction, double angle_start, double angle_end);
		[CCode (cname = "clutter_behaviour_rotate_newx")]
		public BehaviourRotate.newx (Clutter.Alpha alpha, Clutter.RotateAxis axis, Clutter.RotateDirection direction, Clutter.Fixed angle_start, Clutter.Fixed angle_end);
		public void set_axis (Clutter.RotateAxis axis);
		public void set_bounds (double angle_start, double angle_end);
		public void set_boundsx (Clutter.Fixed angle_start, Clutter.Fixed angle_end);
		public void set_center (int x, int y, int z);
		public void set_direction (Clutter.RotateDirection direction);
		[NoAccessorMethod]
		public double angle_end { get; set; }
		[NoAccessorMethod]
		public double angle_start { get; set; }
		public Clutter.RotateAxis axis { get; set; }
		[NoAccessorMethod]
		public int center_x { get; set; }
		[NoAccessorMethod]
		public int center_y { get; set; }
		[NoAccessorMethod]
		public int center_z { get; set; }
		public Clutter.RotateDirection direction { get; set; }
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class BehaviourScale : Clutter.Behaviour {
		public void get_bounds (out double x_scale_start, out double y_scale_start, out double x_scale_end, out double y_scale_end);
		public void get_boundsx (out Clutter.Fixed x_scale_start, out Clutter.Fixed y_scale_start, out Clutter.Fixed x_scale_end, out Clutter.Fixed y_scale_end);
		public BehaviourScale (Clutter.Alpha alpha, double x_scale_start, double y_scale_start, double x_scale_end, double y_scale_end);
		[CCode (cname = "clutter_behaviour_scale_newx")]
		public BehaviourScale.newx (Clutter.Alpha alpha, Clutter.Fixed x_scale_start, Clutter.Fixed y_scale_start, Clutter.Fixed x_scale_end, Clutter.Fixed y_scale_end);
		public void set_bounds (double x_scale_start, double y_scale_start, double x_scale_end, double y_scale_end);
		public void set_boundsx (Clutter.Fixed x_scale_start, Clutter.Fixed y_scale_start, Clutter.Fixed x_scale_end, Clutter.Fixed y_scale_end);
		[NoAccessorMethod]
		public double x_scale_end { get; set; }
		[NoAccessorMethod]
		public double x_scale_start { get; set; }
		[NoAccessorMethod]
		public double y_scale_end { get; set; }
		[NoAccessorMethod]
		public double y_scale_start { get; set; }
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public abstract class ChildMeta : GLib.Object {
		public weak Clutter.Actor get_actor ();
		public weak Clutter.Container get_container ();
		public Clutter.Actor actor { get; set construct; }
		public Clutter.Container container { get; set construct; }
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class CloneTexture : Clutter.Actor, Clutter.Scriptable {
		public weak Clutter.Texture get_parent_texture ();
		public CloneTexture (Clutter.Texture texture);
		public void set_parent_texture (Clutter.Texture texture);
		public Clutter.Texture parent_texture { get; set; }
		[NoAccessorMethod]
		public bool repeat_x { get; set; }
		[NoAccessorMethod]
		public bool repeat_y { get; set; }
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class EffectTemplate : GLib.Object {
		public void @construct (Clutter.Timeline timeline, Clutter.AlphaFunc alpha_func, GLib.DestroyNotify notify);
		public bool get_timeline_clone ();
		public EffectTemplate (Clutter.Timeline timeline, Clutter.AlphaFunc alpha_func);
		public EffectTemplate.for_duration (uint msecs, Clutter.AlphaFunc alpha_func);
		public EffectTemplate.full (Clutter.Timeline timeline, Clutter.AlphaFunc alpha_func, GLib.DestroyNotify notify);
		public void set_timeline_clone (bool setting);
		[NoAccessorMethod]
		public bool clone { get; set; }
		[NoAccessorMethod]
		public Clutter.Timeline timeline { get; construct; }
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class Entry : Clutter.Actor, Clutter.Scriptable {
		public void delete_chars (uint len);
		public void delete_text (long start_pos, long end_pos);
		public Pango.Alignment get_alignment ();
		public void get_color (Clutter.Color color);
		public int get_cursor_position ();
		public weak string get_font_name ();
		public unichar get_invisible_char ();
		public weak Pango.Layout get_layout ();
		public int get_max_length ();
		public weak string get_text ();
		public bool get_visibility ();
		public bool get_visible_cursor ();
		public void insert_text (string text, long position);
		public void insert_unichar (unichar wc);
		public Entry ();
		public Entry.full (string font_name, string text, Clutter.Color color);
		public Entry.with_text (string font_name, string text);
		public void set_alignment (Pango.Alignment alignment);
		public void set_color (Clutter.Color color);
		public void set_cursor_position (int position);
		public void set_font_name (string font_name);
		public void set_invisible_char (unichar wc);
		public void set_max_length (int max);
		public void set_text (string text);
		public void set_visibility (bool visible);
		public void set_visible_cursor (bool visible);
		[NoWrapper]
		public virtual void paint_cursor ();
		public Pango.Alignment alignment { get; set; }
		public Clutter.Color color { get; set; }
		[NoAccessorMethod]
		public bool cursor_visible { get; set; }
		[NoAccessorMethod]
		public uint entry_padding { get; set; }
		public string font_name { get; set; }
		public int max_length { get; set; }
		[NoAccessorMethod]
		public int position { get; set; }
		public string text { get; set; }
		[NoAccessorMethod]
		public bool text_visible { get; set; }
		[NoAccessorMethod]
		public double x_align { get; set; }
		public virtual signal void activate ();
		public virtual signal void cursor_event (Clutter.Geometry geometry);
		public virtual signal void text_changed ();
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class Group : Clutter.Actor, Clutter.Scriptable, Clutter.Container {
		public int get_n_children ();
		public weak Clutter.Actor get_nth_child (int index_);
		public Group ();
		public void remove_all ();
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class Label : Clutter.Actor, Clutter.Scriptable {
		public Pango.Alignment get_alignment ();
		public weak Pango.AttrList get_attributes ();
		public void get_color (out Clutter.Color color);
		public Pango.EllipsizeMode get_ellipsize ();
		public weak string get_font_name ();
		public bool get_justify ();
		public weak Pango.Layout get_layout ();
		public bool get_line_wrap ();
		public Pango.WrapMode get_line_wrap_mode ();
		public weak string get_text ();
		public bool get_use_markup ();
		public Label ();
		public Label.full (string font_name, string text, Clutter.Color color);
		public Label.with_text (string font_name, string text);
		public void set_alignment (Pango.Alignment alignment);
		public void set_attributes (Pango.AttrList attrs);
		public void set_color (Clutter.Color color);
		public void set_ellipsize (Pango.EllipsizeMode mode);
		public void set_font_name (string font_name);
		public void set_justify (bool justify);
		public void set_line_wrap (bool wrap);
		public void set_line_wrap_mode (Pango.WrapMode wrap_mode);
		public void set_text (string text);
		public void set_use_markup (bool setting);
		public Pango.Alignment alignment { get; set; }
		public Pango.AttrList attributes { get; set; }
		public Clutter.Color color { get; set; }
		public Pango.EllipsizeMode ellipsize { get; set; }
		public string font_name { get; set; }
		public bool justify { get; set; }
		public string text { get; set; }
		public bool use_markup { get; set; }
                [NoAccessorMethod]
		public bool wrap { get; set; }
                [NoAccessorMethod]
		public Pango.WrapMode wrap_mode { get; set; }
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class ListModel : Clutter.Model {
		public ListModel (uint n_columns, ...);
		[CCode (cname = "clutter_list_model_newv")]
		public ListModel.newv (uint n_columns, GLib.Type[] types, string[] names);
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public abstract class Model : GLib.Object {
                [CCode (sentinel = "-1")]
		public void append (...);
		public void appendv (uint n_columns, uint columns, GLib.Value values);
		public bool filter_iter (Clutter.ModelIter iter);
		public bool filter_row (uint row);
		public void @foreach (Clutter.ModelForeachFunc func);
		public weak Clutter.ModelIter get_first_iter ();
		public weak Clutter.ModelIter get_last_iter ();
		public int get_sorting_column ();
                [CCode (sentinel = "-1")]
		public void insert (uint row, ...);
		public void insert_value (uint row, uint column, GLib.Value value);
		public void insertv (uint row, uint n_columns, uint columns, GLib.Value values);
                [CCode (sentinel = "-1")]
		public void prepend (...);
		public void prependv (uint n_columns, uint columns, GLib.Value values);
		public void remove (uint row);
		public void set_filter (Clutter.ModelFilterFunc func, GLib.DestroyNotify notify);
		public void set_names (uint n_columns, string[] names);
		public void set_sort (uint column, Clutter.ModelSortFunc func, GLib.DestroyNotify notify);
		public void set_sorting_column (int column);
		public void set_types (uint n_columns, GLib.Type[] types);
		public virtual weak string get_column_name (uint column);
		public virtual GLib.Type get_column_type (uint column);
		public virtual weak Clutter.ModelIter get_iter_at_row (uint row);
		public virtual uint get_n_columns ();
		public virtual uint get_n_rows ();
		[NoWrapper]
		public virtual weak Clutter.ModelIter insert_row (int index_);
		[NoWrapper]
		public virtual void remove_row (uint row);
		public virtual void resort ();
		public virtual signal void filter_changed ();
		public virtual signal void row_added (Clutter.ModelIter iter);
		public virtual signal void row_changed (Clutter.ModelIter iter);
		public virtual signal void row_removed (Clutter.ModelIter iter);
		public virtual signal void sort_changed ();
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public abstract class ModelIter : GLib.Object {
                [CCode (sentinel = "-1")]
		public void @get (...);
		[CCode (sentinel = "-1")]
		public void @set (...);
		public virtual weak Clutter.ModelIter copy ();
		public virtual weak Clutter.Model get_model ();
		public virtual uint get_row ();
		public virtual void get_value (uint column, GLib.Value value);
		public virtual bool is_first ();
		public virtual bool is_last ();
		public virtual weak Clutter.ModelIter next ();
		public virtual weak Clutter.ModelIter prev ();
		public virtual void set_value (uint column, GLib.Value value);
		[NoAccessorMethod]
		public Clutter.Model model { get; set; }
		[NoAccessorMethod]
		public uint row { get; set; }
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class Rectangle : Clutter.Actor, Clutter.Scriptable {
		public void get_border_color (out Clutter.Color color);
		public uint get_border_width ();
		public void get_color (out Clutter.Color color);
		public Rectangle ();
		public Rectangle.with_color (Clutter.Color color);
		public void set_border_color (ref Clutter.Color color);
		public void set_border_width (uint width);
		public void set_color (Clutter.Color color);
		public Clutter.Color border_color { get; set; }
		public uint border_width { get; set; }
		public Clutter.Color color { get; set; }
		[NoAccessorMethod]
		public bool has_border { get; set; }
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class Score : GLib.Object {
		public ulong append (Clutter.Timeline parent, Clutter.Timeline timeline);
		public ulong append_at_marker (Clutter.Timeline parent, string marker_name, Clutter.Timeline timeline);
		public bool get_loop ();
		public weak Clutter.Timeline get_timeline (ulong id);
		public bool is_playing ();
		public weak GLib.SList list_timelines ();
		public Score ();
		public void pause ();
		public void remove (ulong id);
		public void remove_all ();
		public void rewind ();
		public void set_loop (bool loop);
		public void start ();
		public void stop ();
		public bool loop { get; set; }
		public virtual signal void completed ();
		public virtual signal void paused ();
		public virtual signal void started ();
		public virtual signal void timeline_completed (Clutter.Timeline timeline);
		public virtual signal void timeline_started (Clutter.Timeline timeline);
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class Script : GLib.Object {
		public void add_search_paths (string[][] paths);
		public void connect_signals (void* user_data);
		public void connect_signals_full (Clutter.ScriptConnectFunc func);
		public void ensure_objects ();
		public static GLib.Quark error_quark ();
		public weak GLib.Object get_object (string name);
		public int get_objects (...);
		public uint load_from_data (string data, long length) throws GLib.Error;
		public uint load_from_file (string filename) throws GLib.Error;
		public weak string lookup_filename (string filename);
		public Script ();
		public void unmerge_objects (uint merge_id);
		public virtual GLib.Type get_type_from_name (string type_name);
		[NoAccessorMethod]
		public string filename { get; }
		[NoAccessorMethod]
		public bool filename_set { get; }
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class Shader : GLib.Object {
		public bool compile () throws GLib.Error;
		public static GLib.Quark error_quark ();
		public weak string get_fragment_source ();
		public bool get_is_enabled ();
		public weak string get_vertex_source ();
		public bool is_compiled ();
		public Shader ();
		public void release ();
		public void set_fragment_source (string data, long length);
		public void set_is_enabled (bool enabled);
		public void set_uniform_1f (string name, float value);
		public void set_vertex_source (string data, long length);
		[NoAccessorMethod]
		public bool compiled { get; }
		[NoAccessorMethod]
		public bool enabled { get; set; }
		public string fragment_source { get; set; }
		public string vertex_source { get; set; }
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class Stage : Clutter.Group, Clutter.Scriptable, Clutter.Container {
		public void ensure_current ();
		public bool event (Clutter.Event event);
		public weak Clutter.Actor get_actor_at_pos (int x, int y);
		public void get_color (out Clutter.Color color);
		public static weak Clutter.Actor get_default ();
		public void get_fog (out double density, out double z_near, out double z_far);
		public void get_fogx (out Clutter.Fog fog);
		public weak Clutter.Actor get_key_focus ();
		public void get_perspective (out float fovy, out float aspect, out float z_near, out float z_far);
		public void get_perspectivex (out Clutter.Perspective perspective);
		public double get_resolution ();
		public Clutter.Fixed get_resolutionx ();
		public weak string get_title ();
		public bool get_use_fog ();
		public bool get_user_resizable ();
		public void hide_cursor ();
		public bool is_default ();
		public Stage ();
		public void queue_redraw ();
		public weak uchar[] read_pixels (int x, int y, int width, int height);
		public void set_color (Clutter.Color color);
		public void set_fog (double density, double z_near, double z_far);
		public void set_fogx (Clutter.Fog fog);
		public void set_key_focus (Clutter.Actor? actor);
		public void set_perspective (float fovy, float aspect, float z_near, float z_far);
		public void set_perspectivex (Clutter.Perspective perspective);
		public void set_title (string title);
		public void set_use_fog (bool fog);
		public void set_user_resizable (bool resizable);
		public void show_cursor ();
		public Clutter.Color color { get; set; }
		[NoAccessorMethod]
		public bool cursor_visible { get; set construct; }
		[NoAccessorMethod]
		public bool offscreen { get; set construct; }
		public string title { get; set; }
		public bool use_fog { get; set; }
		public bool user_resizable { get; set construct; }
		public virtual signal void activate ();
		public virtual signal void deactivate ();
		[HasEmitter]
		public virtual signal void fullscreen ();
		[HasEmitter]
		public virtual signal void unfullscreen ();
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class StageManager : GLib.Object {
		public static weak Clutter.StageManager get_default ();
		public weak Clutter.Stage get_default_stage ();
		public weak GLib.SList list_stages ();
		public void set_default_stage (Clutter.Stage stage);
		public Clutter.Stage default_stage { get; set; }
		public virtual signal void stage_added (Clutter.Stage stage);
		public virtual signal void stage_removed (Clutter.Stage stage);
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class Texture : Clutter.Actor, Clutter.Scriptable {
		public static GLib.Quark error_quark ();
		public void get_base_size (out int width, out int height);
		public void* get_cogl_texture ();
		public Clutter.TextureQuality get_filter_quality ();
		public int get_max_tile_waste ();
		public Texture ();
		public Texture.from_actor (Clutter.Actor actor);
		public Texture.from_file (string filename) throws GLib.Error;
		public bool set_area_from_rgb_data (uchar[] data, bool has_alpha, int x, int y, int width, int height, int rowstride, int bpp, Clutter.TextureFlags flags) throws GLib.Error;
		public void set_cogl_texture (Cogl.Handle cogl_tex);
		public void set_filter_quality (Clutter.TextureQuality filter_quality);
		public bool set_from_file (string filename) throws GLib.Error;
		public bool set_from_rgb_data (uchar[] data, bool has_alpha, int width, int height, int rowstride, int bpp, Clutter.TextureFlags flags) throws GLib.Error;
		public bool set_from_yuv_data (uchar[] data, int width, int height, Clutter.TextureFlags flags) throws GLib.Error;
		public void set_max_tile_waste (int max_tile_waste);
		public Clutter.TextureHandle cogl_texture { get; set; }
		[NoAccessorMethod]
		public bool disable_slicing { get; construct; }
		[NoAccessorMethod]
		public string filename { set; }
		public Clutter.TextureQuality filter_quality { get; set construct; }
		[NoAccessorMethod]
		public bool keep_aspect_ratio { get; set; }
		[NoAccessorMethod]
		public int pixel_format { get; }
		[NoAccessorMethod]
		public bool repeat_x { get; set; }
		[NoAccessorMethod]
		public bool repeat_y { get; set; }
		[NoAccessorMethod]
		public bool sync_size { get; set; }
		[NoAccessorMethod]
		public int tile_waste { get; construct; }
		public virtual signal void pixbuf_change ();
		public virtual signal void size_change (int width, int height);
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public class Timeline : GLib.Object {
		public void add_marker_at_frame (string marker_name, uint frame_num);
		public void add_marker_at_time (string marker_name, uint msecs);
		public void advance (uint frame_num);
		public void advance_to_marker (string marker_name);
		public weak Clutter.Timeline clone ();
		public int get_current_frame ();
		public uint get_delay ();
		public uint get_delta (out uint msecs);
		public Clutter.TimelineDirection get_direction ();
		public uint get_duration ();
		public bool get_loop ();
		public uint get_n_frames ();
		public double get_progress ();
		public Clutter.Fixed get_progressx ();
		public uint get_speed ();
		public bool has_marker (string marker_name);
		public bool is_playing ();
		public weak string[] list_markers (int frame_num);
		public Timeline (uint n_frames, uint fps);
		public Timeline.for_duration (uint msecs);
		public void pause ();
		public void remove_marker (string marker_name);
		public void rewind ();
		public void set_delay (uint msecs);
		public void set_direction (Clutter.TimelineDirection direction);
		public void set_duration (uint msecs);
		public void set_loop (bool loop);
		public void set_n_frames (uint n_frames);
		public void set_speed (uint fps);
		public void skip (uint n_frames);
		public void start ();
		public void stop ();
		public uint delay { get; set; }
		public Clutter.TimelineDirection direction { get; set; }
		public uint duration { get; set; }
		[NoAccessorMethod]
		public uint fps { get; set; }
		public bool loop { get; set; }
		[NoAccessorMethod]
		public uint num_frames { get; set; }
		public virtual signal void completed ();
		public virtual signal void marker_reached (string marker_name, int frame_num);
		public virtual signal void new_frame (int frame_num);
		public virtual signal void paused ();
		public virtual signal void started ();
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public interface Scriptable {
		public abstract weak string get_id ();
		public abstract bool parse_custom_node (Clutter.Script script, GLib.Value value, string name, void* node);
		public abstract void set_custom_property (Clutter.Script script, string name, GLib.Value value);
		public abstract void set_id (string id);
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public interface Container : GLib.Object {
		public void add_actor (Clutter.Actor actor);
		public void child_get (Clutter.Actor actor, ...);
		public void child_get_property (Clutter.Actor child, string property, GLib.Value value);
		public void child_set (Clutter.Actor actor, ...);
		public void child_set_property (Clutter.Actor child, string property, GLib.Value value);
		[CCode (cname = "clutter_container_class_find_child_property")]
		public class weak GLib.ParamSpec find_child_property (string property_name);
		[CCode (cname = "clutter_container_class_list_child_properties")]
		public class weak GLib.ParamSpec[] list_child_properties ();
                public class GLib.Type child_meta_type;
		public weak Clutter.Actor find_child_by_name (string child_name);
		public weak GLib.List get_children ();
		public void lower_child (Clutter.Actor actor, Clutter.Actor sibling);
		public void raise_child (Clutter.Actor actor, Clutter.Actor sibling);
		public void remove_actor (Clutter.Actor actor);
		public abstract void add (Clutter.Actor actor, ...);
		[NoWrapper]
		public abstract void create_child_meta (Clutter.Actor actor);
		[NoWrapper]
		public abstract void destroy_child_meta (Clutter.Actor actor);
		public abstract void @foreach (Clutter.Callback callback);
		public abstract weak Clutter.ChildMeta get_child_meta (Clutter.Actor actor);
		[NoWrapper]
		public abstract void lower (Clutter.Actor actor, Clutter.Actor? sibling);
		[NoWrapper]
		public abstract void raise (Clutter.Actor actor, Clutter.Actor? sibling);
		public abstract void remove (Clutter.Actor actor, ...);
		public abstract void sort_depth_order ();
		public virtual signal void actor_added (Clutter.Actor actor);
		public virtual signal void actor_removed (Clutter.Actor actor);
		public virtual signal void child_notify (Clutter.Actor actor, GLib.ParamSpec pspec);
	}

	[CCode (cheader_filename = "clutter/clutter.h")]
	public interface Media {
		public bool get_can_seek ();
		public void set_filename (string filename);
		public abstract int get_buffer_percent ();
		public abstract int get_duration ();
		public abstract bool get_playing ();
		public abstract int get_position ();
		public abstract weak string get_uri ();
		public abstract double get_volume ();
		public abstract void set_playing (bool playing);
		public abstract void set_position (int position);
		public abstract void set_uri (string uri);
		public abstract void set_volume (double volume);
		public virtual signal void eos ();
		public virtual signal void error (void* error);
	}

	[SimpleType]
        [CCode (cname = "ClutterUnit", cheader_filename = "clutter/clutter.h", type_id = "CLUTTER_TYPE_UNIT", marshaller_type_name = "INT", get_value_function = "clutter_value_get_unit", set_value_function = "clutter_value_set_unit", default_value = "0", type_signature = "i")]
        [IntegerType (rank = 6)]
	public struct Unit {
		[CCode (cname = "CLUTTER_MINUNIT")]
		public Clutter.Unit MIN;
		[CCode (cname = "CLUTTER_MAXUNIT")]
		public Clutter.Unit MAX;
		[CCode (cname = "g_strdup_printf", instance_pos = -1)]
		public string to_string (string format = "%i");
		[CCode (cname = "MIN")]
		public static Clutter.Unit min (Clutter.Unit a, Clutter.Unit b);
		[CCode (cname = "MAX")]
		public static Clutter.Unit max (Clutter.Unit a, Clutter.Unit b);
		[CCode (cname = "CLAMP")]
		public Clutter.Unit clamp (Clutter.Unit low, Clutter.Unit high);
		[CCode (cname = "CLUTTER_UNITS_FROM_DEVICE")]
		public static Clutter.Unit from_device (int pixels);
		[CCode (cname = "CLUTTER_UNITS_TO_DEVICE")]
		public int to_device ();
		[CCode (cname = "CLUTTER_UNITS_FROM_INT")]
		public static Clutter.Unit from_int (int value_);
		[CCode (cname = "CLUTTER_UNITS_TO_INT")]
		public int to_int ();
		[CCode (cname = "CLUTTER_UNITS_FROM_FLOAT")]
		public static Clutter.Unit from_float (float value_);
		[CCode (cname = "CLUTTER_UNITS_TO_FLOAT")]
		public float to_float ();
		[CCode (cname = "CLUTTER_UNITS_FROM_FIXED")]
		public static Clutter.Unit from_fixed (Clutter.Fixed value_);
		[CCode (cname = "CLUTTER_UNITS_TO_FIXED")]
		public Clutter.Fixed to_fixed ();
	}

	[SimpleType]
        [CCode (cname = "ClutterFixed", cheader_filename = "clutter/clutter.h", type_id = "CLUTTER_TYPE_FIXED", get_value_function = "clutter_value_get_fixed", set_value_function = "clutter_value_set_fixed", default_value = "0", type_signature = "i", marshaller_type_name = "INT")]
        [IntegerType (rank = 6)]
	public struct Fixed {
                [CCode (cname = "CFX_ONE")]
                public const Clutter.Fixed ONE;
		[CCode (cname = "CFX_MIN")]
		public const Clutter.Fixed MIN;
		[CCode (cname = "CFX_MAX")]
		public const Clutter.Fixed MAX;
		[CCode (cname = "CFX_PI")]
		public const Clutter.Fixed PI;
		[CCode (cname = "CFX_2PI")]
		public const Clutter.Fixed 2PI;
		[CCode (cname = "CFX_PI_2")]
		public const Clutter.Fixed PI_2;
		[CCode (cname = "CFX_PI_4")]
		public const Clutter.Fixed PI_4;
                [CCode (cname = "CFX_Q")]
                public const Clutter.Fixed Q;
		[CCode (cname = "g_strdup_printf", instance_pos = -1)]
		public string to_string (string format = "%i");
		[CCode (cname = "MIN")]
		public static Clutter.Fixed min (Clutter.Fixed a, Clutter.Fixed b);
		[CCode (cname = "MAX")]
		public static Clutter.Fixed max (Clutter.Fixed a, Clutter.Fixed b);
		[CCode (cname = "CLAMP")]
		public Clutter.Fixed clamp (Clutter.Fixed low, Clutter.Fixed high);
		[CCode (cname = "CLUTTER_INT_TO_FIXED")]
		public static Clutter.Fixed from_int (int value_);
		[CCode (cname = "CLUTTER_FIXED_TO_INT")]
		public int to_int ();
		[CCode (cname = "CLUTTER_FLOAT_TO_FIXED")]
		public static Clutter.Fixed from_float (float value_);
		[CCode (cname = "CLUTTER_FIXED_TO_FLOAT")]
		public float to_float ();
		
                [CCode (cname = "CLUTTER_FIXED_FLOOR")]
		public Clutter.Fixed floor ();
		[CCode (cname = "CLUTTER_FIXED_CEIL")]
		public Clutter.Fixed ceil ();
		[CCode (cname = "CLUTTER_FIXED_FRACTION")]
		public Clutter.Fixed frac ();
		
                [CCode (cname = "clutter_fixed_qmul")]
		public static Clutter.Fixed mul (Clutter.Fixed a, Clutter.Fixed b);
		[CCode (cname = "clutter_fixed_qdiv")]
		public static Clutter.Fixed div (Clutter.Fixed a, Clutter.Fixed b);
		
                [CCode (cname = "clutter_sinx")]
		public Clutter.Fixed sin ();
		[CCode (cname = "clutter_cosx")]
		public Clutter.Fixed cos ();
		[CCode (cname = "clutter_atani")]
		public Clutter.Fixed atan ();
		[CCode (cname = "clutter_sqrtx")]
		public Clutter.Fixed sqrt ();
		[CCode (cname = "clutter_pow2x")]
		public Clutter.Fixed pow2 ();
                [CCode (cname = "clutter_atan2i")]
	        public static Clutter.Fixed atan2 (Clutter.Fixed y, Clutter.Fixed x);
                [CCode (cname = "clutter_log2x")]
	        public static Clutter.Fixed log2 (uint x);

                [CCode (cname = "CFX_RADIANS_TO_DEGREES")]
                public const int RADIANS_TO_DEGREES;
	}

	[SimpleType]
	[CCode (cname = "ClutterAngle", cheader_filename = "clutter/clutter.h", type_signature = "i")]
        [IntegerType (rank = 6)]
	public struct Angle {
		[CCode (cname = "g_strdup_printf", instance_pos = -1)]
		public string to_string (string format = "%i");
		[CCode (cname = "MIN")]
		public static Clutter.Angle min (Clutter.Angle a, Clutter.Angle b);
		[CCode (cname = "MAX")]
		public static Clutter.Angle max (Clutter.Angle a, Clutter.Angle b);
		[CCode (cname = "CLAMP")]
		public Clutter.Angle clamp (Clutter.Angle low, Clutter.Angle high);
		[CCode (cname = "CLUTTER_ANGLE_FROM_DEGF")]
		public Clutter.Angle from_degf (float value_);
		[CCode (cname = "CLUTTER_ANGLE_FROM_DEGX")]
		public Clutter.Angle from_degx (Clutter.Fixed value_);
		[CCode (cname = "CLUTTER_ANGLE_TO_DEGF")]
		public float to_degf ();
		[CCode (cname = "CLUTTER_ANGLE_TO_DEGX")]
		public Clutter.Fixed to_degx ();
		[CCode (cname = "clutter_sini")]
		public Clutter.Fixed sin ();
		[CCode (cname = "clutter_cosi")]
		public Clutter.Fixed cos ();
		[CCode (cname = "clutter_tani")]
		public Clutter.Fixed tan ();
	}

	[CCode (type_id = "CLUTTER_TYPE_ACTOR_BOX", cheader_filename = "clutter/clutter.h")]
	public struct ActorBox {
		public Clutter.Unit x1;
		public Clutter.Unit y1;
		public Clutter.Unit x2;
		public Clutter.Unit y2;

		public static void get_from_vertices (Clutter.Vertex[] vtx, out Clutter.ActorBox box);
	}

	[CCode (type_id = "CLUTTER_TYPE_COLOR", copy_func = "clutter_color_copy", cheader_filename = "clutter/clutter.h")]
	public struct Color {
		public uchar red;
		public uchar green;
		public uchar blue;
		public uchar alpha;

		public void add (Clutter.Color src2, out Clutter.Color dest);

		public Clutter.Color copy ();

		public void darken (out Clutter.Color dest);
		public bool equal (Clutter.Color b);
		public void free ();
		public void from_hls (uchar hue, uchar luminance, uchar saturation);
		public void from_hlsx (Clutter.Fixed hue, Clutter.Fixed luminance, Clutter.Fixed saturation);
		public void from_pixel (uint pixel);
		public void lighten (out Clutter.Color dest);
		public static bool parse (string color, out Clutter.Color dest);
		public void shade (double shade);
		public void shadex (Clutter.Fixed shade);
		public void subtract (Clutter.Color src2, out Clutter.Color dest);
		public void to_hls (out uchar hue, out uchar luminance, out uchar saturation);
		public void to_hlsx (out Clutter.Fixed hue, out Clutter.Fixed luminance, out Clutter.Fixed saturation);
		public uint to_pixel ();
		public weak string to_string ();
	}

	[CCode (type_id = "CLUTTER_TYPE_FOG", cheader_filename = "clutter/clutter.h")]
	public struct Fog {
		public Clutter.Fixed density;
		public Clutter.Fixed z_near;
		public Clutter.Fixed z_far;
	}

	[CCode (type_id = "CLUTTER_TYPE_KNOT", cheader_filename = "clutter/clutter.h")]
	public struct Knot {
		public int x;
		public int y;
		public Clutter.Knot copy ();
		public bool equal (Clutter.Knot knot_b);
		public void free ();
	}

	[CCode (type_id = "CLUTTER_TYPE_PERSPECTIVE", cheader_filename = "clutter/clutter.h")]
	public struct Perspective {
		public Clutter.Fixed fovy;
		public Clutter.Fixed aspect;
		public Clutter.Fixed z_near;
		public Clutter.Fixed z_far;
	}

	[CCode (type_id = "CLUTTER_TYPE_VERTEX", cheader_filename = "clutter/clutter.h")]
	public struct Vertex {
		public Clutter.Unit x;
		public Clutter.Unit y;
		public Clutter.Unit z;
	}

	public static delegate uint AlphaFunc (Clutter.Alpha alpha, void* user_data);
	public static delegate void EffectCompleteFunc (Clutter.Actor actor, void* user_data);
	public delegate void BehaviourForeachFunc (Clutter.Behaviour behaviour, Clutter.Actor actor);
	public delegate void Callback (Clutter.Actor actor);
	public delegate bool ModelFilterFunc (Clutter.Model model, Clutter.ModelIter iter);
	public delegate bool ModelForeachFunc (Clutter.Model model, Clutter.ModelIter iter);
	public delegate int ModelSortFunc (Clutter.Model model, GLib.Value a, GLib.Value b);
	public delegate void ScriptConnectFunc (Clutter.Script script, GLib.Object object, string signal_name, string handler_name, GLib.Object connect_object, GLib.ConnectFlags flags);
	
        public const string COGL;
	public const int CURRENT_TIME;
	public const string FLAVOUR;
	public const int MAJOR_VERSION;
	public const int MICRO_VERSION;
	public const int MINOR_VERSION;
	public const int PRIORITY_REDRAW;
	public const int PRIORITY_TIMELINE;
	public const int SQRTI_ARG_10_PERCENT;
	public const int SQRTI_ARG_5_PERCENT;
	public const int SQRTI_ARG_MAX;
	public const int VERSION_HEX;
	public const string VERSION_S;

	public static void base_init ();
	public static void clear_glyph_cache ();
	public static void do_event (Clutter.Event event);

        public struct Effect {
                public static weak Clutter.Timeline depth (Clutter.EffectTemplate template_, Clutter.Actor actor, int depth_end, Clutter.EffectCompleteFunc? func, void* data);
                public static weak Clutter.Timeline fade (Clutter.EffectTemplate template_, Clutter.Actor actor, uchar opacity_end, Clutter.EffectCompleteFunc? func, void* data);
	        public static weak Clutter.Timeline move (Clutter.EffectTemplate template_, Clutter.Actor actor, int x, int y, Clutter.EffectCompleteFunc? func, void* data);
        	public static weak Clutter.Timeline path (Clutter.EffectTemplate template_, Clutter.Actor actor, Clutter.Knot[] knots, Clutter.EffectCompleteFunc? func, void* data);
	        public static weak Clutter.Timeline rotate (Clutter.EffectTemplate template_, Clutter.Actor actor, Clutter.RotateAxis axis, double angle, int center_x, int center_y, int center_z, Clutter.RotateDirection direction, Clutter.EffectCompleteFunc? func, void* data);
        	public static weak Clutter.Timeline scale (Clutter.EffectTemplate template_, Clutter.Actor actor, double x_scale_end, double y_scale_end, Clutter.EffectCompleteFunc? func, void* data);
        }

	public static bool events_pending ();

	public static bool feature_available (Clutter.FeatureFlags feature);
	public static Clutter.FeatureFlags feature_get_all ();

        namespace FrameSource {
                public static uint add (uint interval, GLib.SourceFunc func);
                public static uint add_full (int priority, uint interval, GLib.SourceFunc func);
        }
	
	public static weak Clutter.Actor get_actor_by_gid (uint id);
	public static bool get_debug_enabled ();
	
	public static uint get_default_frame_rate ();
	public static weak Clutter.InputDevice get_input_device_for_id (int id);
	public static weak Clutter.Actor get_keyboard_grab ();
	public static bool get_motion_events_enabled ();
	public static uint get_motion_events_frequency ();
	public static weak GLib.OptionGroup get_option_group ();
	public static weak Clutter.Actor get_pointer_grab ();

        public static weak string get_script_id (GLib.Object gobject);

	public static bool get_show_fps ();
	public static ulong get_timestamp ();
	public static bool get_use_mipmapped_text ();
	
        namespace Grab {
                public static void keyboard (Clutter.Actor actor);
                public static void pointer (Clutter.Actor actor);
                public static void pointer_for_device (Clutter.Actor actor, int id);
        }

	public static GLib.Quark init_error_quark ();
	public static Clutter.InitError init ([CCode (array_length_pos = 0.9)] ref weak string[] argv);
	public static Clutter.InitError init_with_args ([CCode (array_length_pos = 0.9)] ref weak string[] argv, string parameter_string, GLib.OptionEntry entries, string translation_domain) throws GLib.Error;

	public static uint keysym_to_unicode (uint keyval);
	
	public static void main ();
	public static int main_level ();
	public static void main_quit ();

	public static void redraw (Clutter.Stage stage);

	public static void set_default_frame_rate (uint frames_per_sec);
	public static void set_motion_events_enabled (bool enable);
	public static void set_motion_events_frequency (uint frequency);
	public static void set_use_mipmapped_text (bool value);

	public static uint exp_dec_func (Clutter.Alpha alpha, void* dummy);
	public static uint exp_inc_func (Clutter.Alpha alpha, void* dummy);
	public static uint ramp_dec_func (Clutter.Alpha alpha, void* dummy);
	public static uint ramp_func (Clutter.Alpha alpha, void* dummy);
	public static uint ramp_inc_func (Clutter.Alpha alpha, void* dummy);
	public static uint sine_dec_func (Clutter.Alpha alpha, void* dummy);
	public static uint sine_func (Clutter.Alpha alpha, void* dummy);
	public static uint sine_half_func (Clutter.Alpha alpha, void* dummy);
	public static uint sine_inc_func (Clutter.Alpha alpha, void* dummy);
	public static uint smoothstep_dec_func (Clutter.Alpha alpha, void* dummy);
	public static uint smoothstep_inc_func (Clutter.Alpha alpha, void* dummy);
	public static uint square_func (Clutter.Alpha alpha, void* dummy);

        namespace Threads {
                public static uint add_frame_source (uint interval, GLib.SourceFunc func);
                public static uint add_frame_source_full (int priority, uint interval, GLib.SourceFunc func);

                public static uint add_idle (GLib.SourceFunc func);
                public static uint add_idle_full (int priority, GLib.SourceFunc func);

                public static uint add_timeout (uint interval, GLib.SourceFunc func);
                public static uint add_timeout_full (int priority, uint interval, GLib.SourceFunc func);

                public static void enter ();
                public static void init ();
                public static void leave ();

                public static void set_lock_functions (GLib.Callback enter_fn, GLib.Callback leave_fn);
        }

        namespace Ungrab {
                public static void keyboard ();
                public static void pointer ();
	        public static void pointer_for_device (int id);
        }

	[CCode (cname = "GValue", type_id = "G_TYPE_VALUE", cheader_filename = "clutter/clutter.h")]
        public struct Value : GLib.Value {
                public Clutter.Fixed get_fixed ();
                public Clutter.Unit get_unit ();
                public void set_fixed (Clutter.Fixed fixed_);
                public void set_unit (Clutter.Unit units);
        }
}

[CCode (cprefix="PangoClutter", lower_case_cprefix="pango_clutter_",
        cheader_filename="clutter/pangoclutter.h")]
namespace Pango.Clutter {
	public class FontMap : Pango.CairoFontMap, Pango.FontMap {
		public FontMap();
		public Pango.Context create_context();
		public void set_resolution(double dpi);
		public void clear_glyph_cache();
		public void set_use_mipmapping(bool value);
		public bool get_use_mipmapping();
	}

	public static void ensure_glyph_cache_for_layout (Pango.Layout layout);
	public static void render_layout_subpixel (Pango.Layout layout, int x, int y, global::Clutter.Color color, int flags);
	public static void render_layout (Pango.Layout layout, int x, int y, global::Clutter.Color color, int flags);
	public static void render_layout_line (Pango.LayoutLine line, int x, int y, global::Clutter.Color color);
}