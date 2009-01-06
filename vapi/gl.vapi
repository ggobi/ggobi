/* gl.vapi
 *
 * Copyright (C) 2008  Matias De la Puente
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 * Author:
 * 	Matias De la Puente <mfpuente.ar@gmail.com>
 */
 
[CCode (lower_case_cprefix ="gl", cheader_filename="GL/gl.h")]
namespace GL
{
	[CCode (cname="GLenum")]
	public struct GLenum : uint { }
	[CCode (cname="GLboolean")]
	public struct GLboolean : bool { }
	[CCode (cname="GLbitfield")]
	public struct GLbitfield : uint { }
	[CCode (cname="GLvoid")]
	public struct GLvoid : void { }
	[CCode (cname="GLbyte")]
	public struct GLbyte : char { }
	[CCode (cname="GLshort")]
	public struct GLshort : short { }
	[CCode (cname="GLint")]
	public struct GLint : int { }
	[CCode (cname="GLubyte")]
	public struct GLubyte : uchar { }
	[CCode (cname="GLushort")]
	public struct GLushort : ushort { }
	[CCode (cname="GLuint")]
	public struct GLuint : uint { }
	[CCode (cname="GLsizei")]
	public struct GLsizei : int { }
	[CCode (cname="GLfloat")]
	[FloatingType (rank = 1)]
	public struct GLfloat : float { }
	[CCode (cname="GLclampf")]
	[FloatingType (rank = 1)]
	public struct GLclampf : float { }
	[CCode (cname="GLdouble")]
	[FloatingType (rank = 2)]
	public struct GLdouble : double { }
	[CCode (cname="GLclampd")]
	[FloatingType (rank = 2)]
	public struct GLclampd : double { }
	
		
	[CCode (lower_case_cprefix="GL_")] 
	namespace Consts {
		// Data Types
		public const GLenum BYTE;
		public const GLenum UNSIGNED_BYTE;
		public const GLenum SHORT;
		public const GLenum UNSIGNED_SHORT;
		public const GLenum INT;
		public const GLenum UNSIGNED_INT;
		public const GLenum FLOAT;
		public const GLenum 2_BYTES;
		public const GLenum 3_BYTES;
		public const GLenum 4_BYTES;
		public const GLenum DOUBLE;
	
		// Primitives
		public const GLenum POINTS;
		public const GLenum LINES;
		public const GLenum LINE_LOOP;
		public const GLenum LINE_STRIP;
		public const GLenum TRIANGLES;
		public const GLenum TRIANGLE_STRIP;
		public const GLenum TRIANGLE_FAN;
		public const GLenum QUADS;
		public const GLenum QUAD_STRIP;
		public const GLenum POLYGON;
	
		// Vertex Arrays
		public const GLenum VERTEX_ARRAY;
		public const GLenum NORMAL_ARRAY;
		public const GLenum COLOR_ARRAY;
		public const GLenum INDEX_ARRAY;
		public const GLenum TEXTURE_COORD_ARRAY;
		public const GLenum EDGE_FLAG_ARRAY;
		public const GLenum VERTEX_ARRAY_SIZE;
		public const GLenum VERTEX_ARRAY_TYPE;
		public const GLenum VERTEX_ARRAY_STRIDE;
		public const GLenum NORMAL_ARRAY_TYPE;
		public const GLenum NORMAL_ARRAY_STRIDE;
		public const GLenum COLOR_ARRAY_SIZE;
		public const GLenum COLOR_ARRAY_TYPE;
		public const GLenum COLOR_ARRAY_STRIDE;
		public const GLenum INDEX_ARRAY_TYPE;
		public const GLenum INDEX_ARRAY_STRIDE;
		public const GLenum TEXTURE_COORD_ARRAY_SIZE;
		public const GLenum TEXTURE_COORD_ARRAY_TYPE;
		public const GLenum TEXTURE_COORD_ARRAY_STRIDE;
		public const GLenum EDGE_FLAG_ARRAY_STRIDE;
		public const GLenum VERTEX_ARRAY_POINTER;
		public const GLenum NORMAL_ARRAY_POINTER;
		public const GLenum COLOR_ARRAY_POINTER;
		public const GLenum INDEX_ARRAY_POINTER;
		public const GLenum TEXTURE_COORD_ARRAY_POINTER;
		public const GLenum EDGE_FLAG_ARRAY_POINTER;
		public const GLenum V2F;
		public const GLenum V3F;
		public const GLenum C4UB_V2F;
		public const GLenum C4UB_V3F;
		public const GLenum C3F_V3F;
		public const GLenum N3F_V3F;
		public const GLenum C4F_N3F_V3F;
		public const GLenum T2F_V3F;
		public const GLenum T4F_V4F;
		public const GLenum T2F_C4UB_V3F;
		public const GLenum T2F_C3F_V3F;
		public const GLenum T2F_N3F_V3F;
		public const GLenum T2F_C4F_N3F_V3F;
		public const GLenum T4F_C4F_N3F_V4F;
	
		// Matrix Mode
		public const GLenum MATRIX_MODE;
		public const GLenum MODELVIEW;
		public const GLenum PROJECTION;
		public const GLenum TEXTURE;
	
		// Points
		public const GLenum POINT_SMOOTH;
		public const GLenum POINT_SIZE;
		public const GLenum POINT_SIZE_GRANULARITY;
		public const GLenum POINT_SIZE_RANGE;
	
		// Lines
		public const GLenum LINE_SMOOTH;
		public const GLenum LINE_STIPPLE;
		public const GLenum LINE_STIPPLE_PATTERN;
		public const GLenum LINE_STIPPLE_REPEAT;
		public const GLenum LINE_WIDTH;
		public const GLenum LINE_WIDTH_GRANULARITY;
		public const GLenum LINE_WIDTH_RANGE;
	
		// Polygons
		public const GLenum POINT;
		public const GLenum LINE;
		public const GLenum FILL;
		public const GLenum CW;
		public const GLenum CCW;
		public const GLenum FRONT;
		public const GLenum BACK;
		public const GLenum POLYGON_MODE;
		public const GLenum POLYGON_SMOOTH;
		public const GLenum POLYGON_STIPPLE;
		public const GLenum EDGE_FLAG;
		public const GLenum CULL_FACE;
		public const GLenum CULL_FACE_MODE;
		public const GLenum FRONT_FACE;
		public const GLenum POLYGON_OFFSET_FACTOR;
		public const GLenum POLYGON_OFFSET_UNITS;
		public const GLenum POLYGON_OFFSET_POINT;
		public const GLenum POLYGON_OFFSET_LINE;
		public const GLenum POLYGON_OFFSET_FILL;
	
		// Display Lists
		public const GLenum COMPILE;
		public const GLenum COMPILE_AND_EXECUTE;
		public const GLenum LIST_BASE;
		public const GLenum LIST_INDEX;
		public const GLenum LIST_MODE;
	
		// Depth Buffer
		public const GLenum NEVER;
		public const GLenum LESS;
		public const GLenum EQUAL;
		public const GLenum LEQUAL;
		public const GLenum GREATER;
		public const GLenum NOTEQUAL;
		public const GLenum GEQUAL;
		public const GLenum ALWAYS;
		public const GLenum DEPTH_TEST;
		public const GLenum DEPTH_BITS;
		public const GLenum DEPTH_CLEAR_VALUE;
		public const GLenum DEPTH_FUNC;
		public const GLenum DEPTH_RANGE;
		public const GLenum DEPTH_WRITEMASK;
		public const GLenum DEPTH_COMPONENT;
	
		// Lighting
		public const GLenum LIGHTING;
		public const GLenum LIGHT0;
		public const GLenum LIGHT1;
		public const GLenum LIGHT2;
		public const GLenum LIGHT3;
		public const GLenum LIGHT4;
		public const GLenum LIGHT5;
		public const GLenum LIGHT6;
		public const GLenum LIGHT7;
		public const GLenum SPOT_EXPONENT;
		public const GLenum SPOT_CUTOFF;
		public const GLenum CONSTANT_ATTENUATION;
		public const GLenum LINEAR_ATTENUATION;
		public const GLenum QUADRATIC_ATTENUATION;
		public const GLenum AMBIENT;
		public const GLenum DIFFUSE;
		public const GLenum SPECULAR;
		public const GLenum SHININESS;
		public const GLenum EMISSION;
		public const GLenum POSITION;
		public const GLenum SPOT_DIRECTION;
		public const GLenum AMBIENT_AND_DIFFUSE;
		public const GLenum COLOR_INDEXES;
		public const GLenum LIGHT_MODEL_TWO_SIDE;
		public const GLenum LIGHT_MODEL_LOCAL_VIEWER;
		public const GLenum LIGHT_MODEL_AMBIENT;
		public const GLenum FRONT_AND_BACK;
		public const GLenum SHADE_MODEL;
		public const GLenum FLAT;
		public const GLenum SMOOTH;
		public const GLenum COLOR_MATERIAL;
		public const GLenum COLOR_MATERIAL_FACE;
		public const GLenum COLOR_MATERIAL_PARAMETER;
		public const GLenum NORMALIZE;
	
		// User Clipping Planes
		public const GLenum CLIP_PLANE0;
		public const GLenum CLIP_PLANE1;
		public const GLenum CLIP_PLANE2;
		public const GLenum CLIP_PLANE3;
		public const GLenum CLIP_PLANE4;
		public const GLenum CLIP_PLANE5;
	
		// Accumulation Buffer
		public const GLenum ACCUM_RED_BITS;
		public const GLenum ACCUM_GREEN_BITS;
		public const GLenum ACCUM_BLUE_BITS;
		public const GLenum ACCUM_ALPHA_BITS;
		public const GLenum ACCUM_CLEAR_VALUE;
		public const GLenum ACCUM;
		public const GLenum ADD;
		public const GLenum LOAD;
		public const GLenum MULT;
		public const GLenum RETURN;
	
		// Alpha Testing
		public const GLenum ALPHA_TEST;
		public const GLenum ALPHA_TEST_REF;
		public const GLenum ALPHA_TEST_FUNC;
	
		// Blending
		public const GLenum BLEND;
		public const GLenum BLEND_SRC;
		public const GLenum BLEND_DST;
		public const GLenum ZERO;
		public const GLenum ONE;
		public const GLenum SRC_COLOR;
		public const GLenum ONE_MINUS_SRC_COLOR;
		public const GLenum SRC_ALPHA;
		public const GLenum ONE_MINUS_SRC_ALPHA;
		public const GLenum DST_ALPHA;
		public const GLenum ONE_MINUS_DST_ALPHA;
		public const GLenum DST_COLOR;
		public const GLenum ONE_MINUS_DST_COLOR;
		public const GLenum SRC_ALPHA_SATURATE;
	
		// Render Mode
		public const GLenum FEEDBACK;
		public const GLenum RENDER;
		public const GLenum SELECT;
	
		// Feedback
		public const GLenum @2D;
		public const GLenum @3D;
		public const GLenum 3D_COLOR;
		public const GLenum 3D_COLOR_TEXTURE;
		public const GLenum 4D_COLOR_TEXTURE;
		public const GLenum POINT_TOKEN;
		public const GLenum LINE_TOKEN;
		public const GLenum LINE_RESET_TOKEN;
		public const GLenum POLYGON_TOKEN;
		public const GLenum BITMAP_TOKEN;
		public const GLenum DRAW_PIXEL_TOKEN;
		public const GLenum COPY_PIXEL_TOKEN;
		public const GLenum PASS_THROUGH_TOKEN;
		public const GLenum FEEDBACK_BUFFER_POINTER;
		public const GLenum FEEDBACK_BUFFER_SIZE;
		public const GLenum FEEDBACK_BUFFER_TYPE;
	
		// Selection Buffer
		public const GLenum SELECTION_BUFFER_POINTER;
		public const GLenum SELECTION_BUFFER_SIZE;
	
		// Fog
		public const GLenum FOG;
		public const GLenum FOG_MODE;
		public const GLenum FOG_DENSITY;
		public const GLenum FOG_COLOR;
		public const GLenum FOG_INDEX;
		public const GLenum FOG_START;
		public const GLenum FOG_END;
		public const GLenum LINEAR;
		public const GLenum EXP;
		public const GLenum EXP2;
	
		// Logic Ops
		public const GLenum LOGIC_OP;
		public const GLenum INDEX_LOGIC_OP;
		public const GLenum COLOR_LOGIC_OP;
		public const GLenum LOGIC_OP_MODE;
		public const GLenum CLEAR;
		public const GLenum SET;
		public const GLenum COPY;
		public const GLenum COPY_INVERTED;
		public const GLenum NOOP;
		public const GLenum INVERT;
		public const GLenum AND;
		public const GLenum NAND;
		public const GLenum OR;
		public const GLenum NOR;
		public const GLenum XOR;
		public const GLenum EQUIV;
		public const GLenum AND_REVERSE;
		public const GLenum AND_INVERTED;
		public const GLenum OR_REVERSE;
		public const GLenum OR_INVERTED;
	
		// Stencil
		public const GLenum STENCIL_BITS;
		public const GLenum STENCIL_TEST;
		public const GLenum STENCIL_CLEAR_VALUE;
		public const GLenum STENCIL_FUNC;
		public const GLenum STENCIL_VALUE_MASK;
		public const GLenum STENCIL_FAIL;
		public const GLenum STENCIL_PASS_DEPTH_FAIL;
		public const GLenum STENCIL_PASS_DEPTH_PASS;
		public const GLenum STENCIL_REF;
		public const GLenum STENCIL_WRITEMASK;
		public const GLenum STENCIL_INDEX;
		public const GLenum KEEP;
		public const GLenum REPLACE;
		public const GLenum INCR;
		public const GLenum DECR;
	
		// Buffers, Pixel Drawing/Reading
		public const GLenum NONE;
		public const GLenum LEFT;
		public const GLenum RIGHT;
		public const GLenum FRONT_LEFT;
		public const GLenum FRONT_RIGHT;
		public const GLenum BACK_LEFT;
		public const GLenum BACK_RIGHT;
		public const GLenum AUX0;
		public const GLenum AUX1;
		public const GLenum AUX2;
		public const GLenum AUX3;
		public const GLenum COLOR_INDEX;
		public const GLenum RED;
		public const GLenum GREEN;
		public const GLenum BLUE;
		public const GLenum ALPHA;
		public const GLenum LUMINANCE;
		public const GLenum LUMINANCE_ALPHA;
		public const GLenum ALPHA_BITS;
		public const GLenum RED_BITS;
		public const GLenum GREEN_BITS;
		public const GLenum BLUE_BITS;
		public const GLenum INDEX_BITS;
		public const GLenum SUBPIXEL_BITS;
		public const GLenum AUX_BUFFERS;
		public const GLenum READ_BUFFER;
		public const GLenum DRAW_BUFFER;
		public const GLenum DOUBLEBUFFER;
		public const GLenum STEREO;
		public const GLenum BITMAP;
		public const GLenum COLOR;
		public const GLenum DEPTH;
		public const GLenum STENCIL;
		public const GLenum DITHER;
		public const GLenum RGB;
		public const GLenum RGBA;
	
		// Implementation Limits
		public const GLenum MAX_LIST_NESTING;
		public const GLenum MAX_EVAL_ORDER;
		public const GLenum MAX_LIGHTS;
		public const GLenum MAX_CLIP_PLANES;
		public const GLenum MAX_TEXTURE_SIZE;
		public const GLenum MAX_PIXEL_MAP_TABLE;
		public const GLenum MAX_ATTRIB_STACK_DEPTH;
		public const GLenum MAX_MODELVIEW_STACK_DEPTH;
		public const GLenum MAX_NAME_STACK_DEPTH;
		public const GLenum MAX_PROJECTION_STACK_DEPTH;
		public const GLenum MAX_TEXTURE_STACK_DEPTH;
		public const GLenum MAX_VIEWPORT_DIMS;
		public const GLenum MAX_CLIENT_ATTRIB_STACK_DEPTH;
	
		// Gets
		public const GLenum ATTRIB_STACK_DEPTH;
		public const GLenum CLIENT_ATTRIB_STACK_DEPTH;
		public const GLenum COLOR_CLEAR_VALUE;
		public const GLenum COLOR_WRITEMASK;
		public const GLenum CURRENT_INDEX;
		public const GLenum CURRENT_COLOR;
		public const GLenum CURRENT_NORMAL;
		public const GLenum CURRENT_RASTER_COLOR;
		public const GLenum CURRENT_RASTER_DISTANCE;
		public const GLenum CURRENT_RASTER_INDEX;
		public const GLenum CURRENT_RASTER_POSITION;
		public const GLenum CURRENT_RASTER_TEXTURE_COORDS;
		public const GLenum CURRENT_RASTER_POSITION_VALID;
		public const GLenum CURRENT_TEXTURE_COORDS;
		public const GLenum INDEX_CLEAR_VALUE;
		public const GLenum INDEX_MODE;
		public const GLenum INDEX_WRITEMASK;
		public const GLenum MODELVIEW_MATRIX;
		public const GLenum MODELVIEW_STACK_DEPTH;
		public const GLenum NAME_STACK_DEPTH;
		public const GLenum PROJECTION_MATRIX;
		public const GLenum PROJECTION_STACK_DEPTH;
		public const GLenum RENDER_MODE;
		public const GLenum RGBA_MODE;
		public const GLenum TEXTURE_MATRIX;
		public const GLenum TEXTURE_STACK_DEPTH;
		public const GLenum VIEWPORT;
	
		// Evaluators
		public const GLenum AUTO_NORMAL;
		public const GLenum MAP1_COLOR_4;
		public const GLenum MAP1_INDEX;
		public const GLenum MAP1_NORMAL;
		public const GLenum MAP1_TEXTURE_COORD_1;
		public const GLenum MAP1_TEXTURE_COORD_2;
		public const GLenum MAP1_TEXTURE_COORD_3;
		public const GLenum MAP1_TEXTURE_COORD_4;
		public const GLenum MAP1_VERTEX_3;
		public const GLenum MAP1_VERTEX_4;
		public const GLenum MAP2_COLOR_4;
		public const GLenum MAP2_INDEX;
		public const GLenum MAP2_NORMAL;
		public const GLenum MAP2_TEXTURE_COORD_1;
		public const GLenum MAP2_TEXTURE_COORD_2;
		public const GLenum MAP2_TEXTURE_COORD_3;
		public const GLenum MAP2_TEXTURE_COORD_4;
		public const GLenum MAP2_VERTEX_3;
		public const GLenum MAP2_VERTEX_4;
		public const GLenum MAP1_GRID_DOMAIN;
		public const GLenum MAP1_GRID_SEGMENTS;
		public const GLenum MAP2_GRID_DOMAIN;
		public const GLenum MAP2_GRID_SEGMENTS;
		public const GLenum COEFF;
		public const GLenum ORDER;
		public const GLenum DOMAIN;
	
		// Hints
		public const GLenum PERSPECTIVE_CORRECTION_HINT;
		public const GLenum POINT_SMOOTH_HINT;
		public const GLenum LINE_SMOOTH_HINT;
		public const GLenum POLYGON_SMOOTH_HINT;
		public const GLenum FOG_HINT;
		public const GLenum DONT_CARE;
		public const GLenum FASTEST;
		public const GLenum NICEST;
	
		// Scissor box
		public const GLenum SCISSOR_BOX;
		public const GLenum SCISSOR_TEST;
	
		// Pixel Mode / Trasnfer
		public const GLenum MAP_COLOR;
		public const GLenum MAP_STENCIL;
		public const GLenum INDEX_SHIFT;
		public const GLenum INDEX_OFFSET;
		public const GLenum RED_SCALE;
		public const GLenum RED_BIAS;
		public const GLenum GREEN_SCALE;
		public const GLenum GREEN_BIAS;
		public const GLenum BLUE_SCALE;
		public const GLenum BLUE_BIAS;
		public const GLenum ALPHA_SCALE;
		public const GLenum ALPHA_BIAS;
		public const GLenum DEPTH_SCALE;
		public const GLenum DEPTH_BIAS;
		public const GLenum PIXEL_MAP_S_TO_S_SIZE;
		public const GLenum PIXEL_MAP_I_TO_I_SIZE;
		public const GLenum PIXEL_MAP_I_TO_R_SIZE;
		public const GLenum PIXEL_MAP_I_TO_G_SIZE;
		public const GLenum PIXEL_MAP_I_TO_B_SIZE;
		public const GLenum PIXEL_MAP_I_TO_A_SIZE;
		public const GLenum PIXEL_MAP_R_TO_R_SIZE;
		public const GLenum PIXEL_MAP_G_TO_G_SIZE;
		public const GLenum PIXEL_MAP_B_TO_B_SIZE;
		public const GLenum PIXEL_MAP_A_TO_A_SIZE;
		public const GLenum PIXEL_MAP_S_TO_S;
		public const GLenum PIXEL_MAP_I_TO_I;
		public const GLenum PIXEL_MAP_I_TO_R;
		public const GLenum PIXEL_MAP_I_TO_G;
		public const GLenum PIXEL_MAP_I_TO_B;
		public const GLenum PIXEL_MAP_I_TO_A;
		public const GLenum PIXEL_MAP_R_TO_R;
		public const GLenum PIXEL_MAP_G_TO_G;
		public const GLenum PIXEL_MAP_B_TO_B;
		public const GLenum PIXEL_MAP_A_TO_A;
		public const GLenum PACK_ALIGNMENT;
		public const GLenum PACK_LSB_FIRST;
		public const GLenum PACK_ROW_LENGTH;
		public const GLenum PACK_SKIP_PIXELS;
		public const GLenum PACK_SKIP_ROWS;
		public const GLenum PACK_SWAP_BYTES;
		public const GLenum UNPACK_ALIGNMENT;
		public const GLenum UNPACK_LSB_FIRST;
		public const GLenum UNPACK_ROW_LENGTH;
		public const GLenum UNPACK_SKIP_PIXELS;
		public const GLenum UNPACK_SKIP_ROWS;
		public const GLenum UNPACK_SWAP_BYTES;
		public const GLenum ZOOM_X;
		public const GLenum ZOOM_Y;
	
		// Texture Mapping
		public const GLenum TEXTURE_ENV;
		public const GLenum TEXTURE_ENV_MODE;
		public const GLenum TEXTURE_1D;
		public const GLenum TEXTURE_2D;
		public const GLenum TEXTURE_WRAP_S;
		public const GLenum TEXTURE_WRAP_T;
		public const GLenum TEXTURE_MAG_FILTER;
		public const GLenum TEXTURE_MIN_FILTER;
		public const GLenum TEXTURE_ENV_COLOR;
		public const GLenum TEXTURE_GEN_S;
		public const GLenum TEXTURE_GEN_T;
		public const GLenum TEXTURE_GEN_MODE;
		public const GLenum TEXTURE_BORDER_COLOR;
		public const GLenum TEXTURE_WIDTH;
		public const GLenum TEXTURE_HEIGHT;
		public const GLenum TEXTURE_BORDER;
		public const GLenum TEXTURE_COMPONENTS;
		public const GLenum TEXTURE_RED_SIZE;
		public const GLenum TEXTURE_GREEN_SIZE;
		public const GLenum TEXTURE_BLUE_SIZE;
		public const GLenum TEXTURE_ALPHA_SIZE;
		public const GLenum TEXTURE_LUMINANCE_SIZE;
		public const GLenum TEXTURE_INTENSITY_SIZE;
		public const GLenum NEAREST_MIPMAP_NEAREST;
		public const GLenum NEAREST_MIPMAP_LINEAR;
		public const GLenum LINEAR_MIPMAP_NEAREST;
		public const GLenum LINEAR_MIPMAP_LINEAR;
		public const GLenum OBJECT_LINEAR;
		public const GLenum OBJECT_PLANE;
		public const GLenum EYE_LINEAR;
		public const GLenum EYE_PLANE;
		public const GLenum SPHERE_MAP;
		public const GLenum DECAL;
		public const GLenum MODULATE;
		public const GLenum NEAREST;
		public const GLenum REPEAT;
		public const GLenum CLAMP;
		public const GLenum S;
		public const GLenum T;
		public const GLenum R;
		public const GLenum Q;
		public const GLenum TEXTURE_GEN_R;
		public const GLenum TEXTURE_GEN_Q;
	
		// Utility
		public const GLenum VENDOR;
		public const GLenum RENDERER;
		public const GLenum VERSION;
		public const GLenum EXTENSIONS;
	
		// Errors
		public const GLenum NO_ERROR;
		public const GLenum INVALID_ENUM;
		public const GLenum INVALID_VALUE;
		public const GLenum INVALID_OPERATION;
		public const GLenum STACK_OVERFLOW;
		public const GLenum STACK_UNDERFLOW;
		public const GLenum OUT_OF_MEMORY;
	
		// glPush/Pop Attib Bits
		public const GLenum CURRENT_BIT;
		public const GLenum POINT_BIT;
		public const GLenum LINE_BIT;
		public const GLenum POLYGON_BIT;
		public const GLenum POLYGON_STIPPLE_BIT;
		public const GLenum PIXEL_MODE_BIT;
		public const GLenum LIGHTING_BIT;
		public const GLenum FOG_BIT;
		public const GLenum DEPTH_BUFFER_BIT;
		public const GLenum ACCUM_BUFFER_BIT;
		public const GLenum STENCIL_BUFFER_BIT;
		public const GLenum VIEWPORT_BIT;
		public const GLenum TRANSFORM_BIT;
		public const GLenum ENABLE_BIT;
		public const GLenum COLOR_BUFFER_BIT;
		public const GLenum HINT_BIT;
		public const GLenum EVAL_BIT;
		public const GLenum LIST_BIT;
		public const GLenum TEXTURE_BIT;
		public const GLenum SCISSOR_BIT;
		public const GLenum ALL_ATTRIB_BITS;
	
		// OpenGL 1.1
		public const GLenum PROXY_TEXTURE_1D;
		public const GLenum PROXY_TEXTURE_2D;
		public const GLenum TEXTURE_PRIORITY;
		public const GLenum TEXTURE_RESIDENT;
		public const GLenum TEXTURE_BINDING_1D;
		public const GLenum TEXTURE_BINDING_2D;
		public const GLenum TEXTURE_INTERNAL_FORMAT;
		public const GLenum ALPHA4;
		public const GLenum ALPHA8;
		public const GLenum ALPHA12;
		public const GLenum ALPHA16;
		public const GLenum LUMINANCE4;
		public const GLenum LUMINANCE8;
		public const GLenum LUMINANCE12;
		public const GLenum LUMINANCE16;
		public const GLenum LUMINANCE4_ALPHA4;
		public const GLenum LUMINANCE6_ALPHA2;
		public const GLenum LUMINANCE8_ALPHA8;
		public const GLenum LUMINANCE12_ALPHA4;
		public const GLenum LUMINANCE12_ALPHA12;
		public const GLenum LUMINANCE16_ALPHA16;
		public const GLenum INTENSITY;
		public const GLenum INTENSITY4;
		public const GLenum INTENSITY8;
		public const GLenum INTENSITY12;
		public const GLenum INTENSITY16;
		public const GLenum R3_G3_B2;
		public const GLenum RGB4;
		public const GLenum RGB5;
		public const GLenum RGB8;
		public const GLenum RGB10;
		public const GLenum RGB12;
		public const GLenum RGB16;
		public const GLenum RGBA2;
		public const GLenum RGBA4;
		public const GLenum RGB5_A1;
		public const GLenum RGBA8;
		public const GLenum RGB10_A2;
		public const GLenum RGBA12;
		public const GLenum RGBA16;
		public const GLenum CLIENT_PIXEL_STORE_BIT;
		public const GLenum CLIENT_VERTEX_ARRAY_BIT;
		public const GLenum ALL_CLIENT_ATTRIB_BITS;
		public const GLenum CLIENT_ALL_ATTRIB_BITS;
		
		// OpenGL 1.2
		public const GLenum RESCALE_NORMAL;
		public const GLenum CLAMP_TO_EDGE;
		public const GLenum MAX_ELEMENTS_VERTICES;
		public const GLenum MAX_ELEMENTS_INDICES;
		public const GLenum BGR;
		public const GLenum BGRA;
		public const GLenum UNSIGNED_BYTE_3_3_2;
		public const GLenum UNSIGNED_BYTE_2_3_3_REV;
		public const GLenum UNSIGNED_SHORT_5_6_5;
		public const GLenum UNSIGNED_SHORT_5_6_5_REV;
		public const GLenum UNSIGNED_SHORT_4_4_4_4;
		public const GLenum UNSIGNED_SHORT_4_4_4_4_REV;
		public const GLenum UNSIGNED_SHORT_5_5_5_1;
		public const GLenum UNSIGNED_SHORT_1_5_5_5_REV;
		public const GLenum UNSIGNED_INT_8_8_8_8;
		public const GLenum UNSIGNED_INT_8_8_8_8_REV;
		public const GLenum UNSIGNED_INT_10_10_10_2;
		public const GLenum UNSIGNED_INT_2_10_10_10_REV;
		public const GLenum LIGHT_MODEL_COLOR_CONTROL;
		public const GLenum SINGLE_COLOR;
		public const GLenum SEPARATE_SPECULAR_COLOR;
		public const GLenum TEXTURE_MIN_LOD;
		public const GLenum TEXTURE_MAX_LOD;
		public const GLenum TEXTURE_BASE_LEVEL;
		public const GLenum TEXTURE_MAX_LEVEL;
		public const GLenum SMOOTH_POINT_SIZE_RANGE;
		public const GLenum SMOOTH_POINT_SIZE_GRANULARITY;
		public const GLenum SMOOTH_LINE_WIDTH_RANGE;
		public const GLenum SMOOTH_LINE_WIDTH_GRANULARITY;
		public const GLenum ALIASED_POINT_SIZE_RANGE;
		public const GLenum ALIASED_LINE_WIDTH_RANGE;
		public const GLenum PACK_SKIP_IMAGES;
		public const GLenum PACK_IMAGE_HEIGHT;
		public const GLenum UNPACK_SKIP_IMAGES;
		public const GLenum UNPACK_IMAGE_HEIGHT;
		public const GLenum TEXTURE_3D;
		public const GLenum PROXY_TEXTURE_3D;
		public const GLenum TEXTURE_DEPTH;
		public const GLenum TEXTURE_WRAP_R;
		public const GLenum MAX_3D_TEXTURE_SIZE;
		public const GLenum TEXTURE_BINDING_3D;
		
		// GL_ARB_imaging
		public const GLenum ARB_imaging;
		public const GLenum CONSTANT_COLOR;
		public const GLenum ONE_MINUS_CONSTANT_COLOR;
		public const GLenum CONSTANT_ALPHA;
		public const GLenum ONE_MINUS_CONSTANT_ALPHA;
		public const GLenum COLOR_TABLE;
		public const GLenum POST_CONVOLUTION_COLOR_TABLE;
		public const GLenum POST_COLOR_MATRIX_COLOR_TABLE;
		public const GLenum PROXY_COLOR_TABLE;
		public const GLenum PROXY_POST_CONVOLUTION_COLOR_TABLE;
		public const GLenum PROXY_POST_COLOR_MATRIX_COLOR_TABLE;
		public const GLenum COLOR_TABLE_SCALE;
		public const GLenum COLOR_TABLE_BIAS;
		public const GLenum COLOR_TABLE_FORMAT;
		public const GLenum COLOR_TABLE_WIDTH;
		public const GLenum COLOR_TABLE_RED_SIZE;
		public const GLenum COLOR_TABLE_GREEN_SIZE;
		public const GLenum COLOR_TABLE_BLUE_SIZE;
		public const GLenum COLOR_TABLE_ALPHA_SIZE;
		public const GLenum COLOR_TABLE_LUMINANCE_SIZE;
		public const GLenum COLOR_TABLE_INTENSITY_SIZE;
		public const GLenum CONVOLUTION_1D;
		public const GLenum CONVOLUTION_2D;
		public const GLenum SEPARABLE_2D;
		public const GLenum CONVOLUTION_BORDER_MODE;
		public const GLenum CONVOLUTION_FILTER_SCALE;
		public const GLenum CONVOLUTION_FILTER_BIAS;
		public const GLenum REDUCE;
		public const GLenum CONVOLUTION_FORMAT;
		public const GLenum CONVOLUTION_WIDTH;
		public const GLenum CONVOLUTION_HEIGHT;
		public const GLenum MAX_CONVOLUTION_WIDTH;
		public const GLenum MAX_CONVOLUTION_HEIGHT;
		public const GLenum POST_CONVOLUTION_RED_SCALE;
		public const GLenum POST_CONVOLUTION_GREEN_SCALE;
		public const GLenum POST_CONVOLUTION_BLUE_SCALE;
		public const GLenum POST_CONVOLUTION_ALPHA_SCALE;
		public const GLenum POST_CONVOLUTION_RED_BIAS;
		public const GLenum POST_CONVOLUTION_GREEN_BIAS;
		public const GLenum POST_CONVOLUTION_BLUE_BIAS;
		public const GLenum POST_CONVOLUTION_ALPHA_BIAS;
		public const GLenum CONSTANT_BORDER;
		public const GLenum REPLICATE_BORDER;
		public const GLenum CONVOLUTION_BORDER_COLOR;
		public const GLenum COLOR_MATRIX;
		public const GLenum COLOR_MATRIX_STACK_DEPTH;
		public const GLenum MAX_COLOR_MATRIX_STACK_DEPTH;
		public const GLenum POST_COLOR_MATRIX_RED_SCALE;
		public const GLenum POST_COLOR_MATRIX_GREEN_SCALE;
		public const GLenum POST_COLOR_MATRIX_BLUE_SCALE;
		public const GLenum POST_COLOR_MATRIX_ALPHA_SCALE;
		public const GLenum POST_COLOR_MATRIX_RED_BIAS;
		public const GLenum POST_COLOR_MATRIX_GREEN_BIAS;
		public const GLenum POST_COLOR_MATRIX_BLUE_BIAS;
		public const GLenum POST_COLOR_MATRIX_ALPHA_BIAS;
		public const GLenum HISTOGRAM;
		public const GLenum PROXY_HISTOGRAM;
		public const GLenum HISTOGRAM_WIDTH;
		public const GLenum HISTOGRAM_FORMAT;
		public const GLenum HISTOGRAM_RED_SIZE;
		public const GLenum HISTOGRAM_GREEN_SIZE;
		public const GLenum HISTOGRAM_BLUE_SIZE;
		public const GLenum HISTOGRAM_ALPHA_SIZE;
		public const GLenum HISTOGRAM_LUMINANCE_SIZE;
		public const GLenum HISTOGRAM_SINK;
		public const GLenum MINMAX;
		public const GLenum MINMAX_FORMAT;
		public const GLenum MINMAX_SINK;
		public const GLenum TABLE_TOO_LARGE;
		public const GLenum BLEND_EQUATION;
		public const GLenum MIN;
		public const GLenum MAX;
		public const GLenum FUNC_ADD;
		public const GLenum FUNC_SUBTRACT;
		public const GLenum FUNC_REVERSE_SUBTRACT;
		public const GLenum BLEND_COLOR;
		
		// OpenGL 1.3
		public const GLenum TEXTURE0;
		public const GLenum TEXTURE1;
		public const GLenum TEXTURE2;
		public const GLenum TEXTURE3;
		public const GLenum TEXTURE4;
		public const GLenum TEXTURE5;
		public const GLenum TEXTURE6;
		public const GLenum TEXTURE7;
		public const GLenum TEXTURE8;
		public const GLenum TEXTURE9;
		public const GLenum TEXTURE10;
		public const GLenum TEXTURE11;
		public const GLenum TEXTURE12;
		public const GLenum TEXTURE13;
		public const GLenum TEXTURE14;
		public const GLenum TEXTURE15;
		public const GLenum TEXTURE16;
		public const GLenum TEXTURE17;
		public const GLenum TEXTURE18;
		public const GLenum TEXTURE19;
		public const GLenum TEXTURE20;
		public const GLenum TEXTURE21;
		public const GLenum TEXTURE22;
		public const GLenum TEXTURE23;
		public const GLenum TEXTURE24;
		public const GLenum TEXTURE25;
		public const GLenum TEXTURE26;
		public const GLenum TEXTURE27;
		public const GLenum TEXTURE28;
		public const GLenum TEXTURE29;
		public const GLenum TEXTURE30;
		public const GLenum TEXTURE31;
		public const GLenum ACTIVE_TEXTURE;
		public const GLenum CLIENT_ACTIVE_TEXTURE;
		public const GLenum MAX_TEXTURE_UNITS;
		public const GLenum NORMAL_MAP;
		public const GLenum REFLECTION_MAP;
		public const GLenum TEXTURE_CUBE_MAP;
		public const GLenum TEXTURE_BINDING_CUBE_MAP;
		public const GLenum TEXTURE_CUBE_MAP_POSITIVE_X;
		public const GLenum TEXTURE_CUBE_MAP_NEGATIVE_X;
		public const GLenum TEXTURE_CUBE_MAP_POSITIVE_Y;
		public const GLenum TEXTURE_CUBE_MAP_NEGATIVE_Y;
		public const GLenum TEXTURE_CUBE_MAP_POSITIVE_Z;
		public const GLenum TEXTURE_CUBE_MAP_NEGATIVE_Z;
		public const GLenum PROXY_TEXTURE_CUBE_MAP;
		public const GLenum MAX_CUBE_MAP_TEXTURE_SIZE;
		public const GLenum COMPRESSED_ALPHA;
		public const GLenum COMPRESSED_LUMINANCE;
		public const GLenum COMPRESSED_LUMINANCE_ALPHA;
		public const GLenum COMPRESSED_INTENSITY;
		public const GLenum COMPRESSED_RGB;
		public const GLenum COMPRESSED_RGBA;
		public const GLenum TEXTURE_COMPRESSION_HINT;
		public const GLenum TEXTURE_COMPRESSED_IMAGE_SIZE;
		public const GLenum TEXTURE_COMPRESSED;
		public const GLenum NUM_COMPRESSED_TEXTURE_FORMATS;
		public const GLenum COMPRESSED_TEXTURE_FORMATS;
		public const GLenum MULTISAMPLE;
		public const GLenum SAMPLE_ALPHA_TO_COVERAGE;
		public const GLenum SAMPLE_ALPHA_TO_ONE;
		public const GLenum SAMPLE_COVERAGE;
		public const GLenum SAMPLE_BUFFERS;
		public const GLenum SAMPLES;
		public const GLenum SAMPLE_COVERAGE_VALUE;
		public const GLenum SAMPLE_COVERAGE_INVERT;
		public const GLenum MULTISAMPLE_BIT;
		public const GLenum TRANSPOSE_MODELVIEW_MATRIX;
		public const GLenum TRANSPOSE_PROJECTION_MATRIX;
		public const GLenum TRANSPOSE_TEXTURE_MATRIX;
		public const GLenum TRANSPOSE_COLOR_MATRIX;
		public const GLenum COMBINE;
		public const GLenum COMBINE_RGB;
		public const GLenum COMBINE_ALPHA;
		public const GLenum SOURCE0_RGB;
		public const GLenum SOURCE1_RGB;
		public const GLenum SOURCE2_RGB;
		public const GLenum SOURCE0_ALPHA;
		public const GLenum SOURCE1_ALPHA;
		public const GLenum SOURCE2_ALPHA;
		public const GLenum OPERAND0_RGB;
		public const GLenum OPERAND1_RGB;
		public const GLenum OPERAND2_RGB;
		public const GLenum OPERAND0_ALPHA;
		public const GLenum OPERAND1_ALPHA;
		public const GLenum OPERAND2_ALPHA;
		public const GLenum RGB_SCALE;
		public const GLenum ADD_SIGNED;
		public const GLenum INTERPOLATE;
		public const GLenum SUBTRACT;
		public const GLenum CONSTANT;
		public const GLenum PRIMARY_COLOR;
		public const GLenum PREVIOUS;
		public const GLenum DOT3_RGB;
		public const GLenum DOT3_RGBA;
		public const GLenum CLAMP_TO_BORDER;
		
		// GL_ARB_multitexture (ARB extension 1 and OpenGL 1.2.1)
		public const GLenum TEXTURE0_ARB;
		public const GLenum TEXTURE1_ARB;
		public const GLenum TEXTURE2_ARB;
		public const GLenum TEXTURE3_ARB;
		public const GLenum TEXTURE4_ARB;
		public const GLenum TEXTURE5_ARB;
		public const GLenum TEXTURE6_ARB;
		public const GLenum TEXTURE7_ARB;
		public const GLenum TEXTURE8_ARB;
		public const GLenum TEXTURE9_ARB;
		public const GLenum TEXTURE10_ARB;
		public const GLenum TEXTURE11_ARB;
		public const GLenum TEXTURE12_ARB;
		public const GLenum TEXTURE13_ARB;
		public const GLenum TEXTURE14_ARB;
		public const GLenum TEXTURE15_ARB;
		public const GLenum TEXTURE16_ARB;
		public const GLenum TEXTURE17_ARB;
		public const GLenum TEXTURE18_ARB;
		public const GLenum TEXTURE19_ARB;
		public const GLenum TEXTURE20_ARB;
		public const GLenum TEXTURE21_ARB;
		public const GLenum TEXTURE22_ARB;
		public const GLenum TEXTURE23_ARB;
		public const GLenum TEXTURE24_ARB;
		public const GLenum TEXTURE25_ARB;
		public const GLenum TEXTURE26_ARB;
		public const GLenum TEXTURE27_ARB;
		public const GLenum TEXTURE28_ARB;
		public const GLenum TEXTURE29_ARB;
		public const GLenum TEXTURE30_ARB;
		public const GLenum TEXTURE31_ARB;
		public const GLenum ACTIVE_TEXTURE_ARB;
		public const GLenum CLIENT_ACTIVE_TEXTURE_ARB;
		public const GLenum MAX_TEXTURE_UNITS_ARB;
	}
	
	// Miscellaneous
	public static void ClearIndex (GLfloat c);
	public static void ClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	public static void Clear (GLbitfield mask);
	public static void IndexMask (GLuint mask);
	public static void ColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
	public static void AlphaFunc (GLenum func, GLclampf @ref);
	public static void BlendFunc (GLenum sfactor, GLenum dfactor);
	public static void LogicOp (GLenum opcode);
	public static void CullFace (GLenum mode);
	public static void FrontFace (GLenum mode);
	public static void PointSize (GLfloat size);
	public static void LineWidth (GLfloat width);
	public static void LineStipple (GLint factor, GLushort pattern);
	public static void PolygonMode (GLenum face, GLenum mode);
	public static void PolygonOffset (GLfloat factor, GLfloat units);
	[CCode (array_length = false)]
	public static void PolygonStipple (GLubyte[] mask);
	public static void GetPolygonStipple (out GLubyte mask);
	public static void EdgeFlag (GLboolean flag);
	[CCode (array_length = false)]
	public static void EdgeFlagv (GLboolean[] flag);
	public static void Scissor (GLint x, GLint y, GLsizei width, GLsizei height);
	[CCode (array_length = false)]
	public static void ClipPlane (GLenum plane, GLdouble[] equation);
	public static void GetClipPlane (GLenum plane, out GLdouble equation);
	public static void DrawBuffer (GLenum mode);
	public static void ReadBuffer (GLenum mode);
	public static void Enable (GLenum cap);
	public static void Disable (GLenum cap);
	public static GLboolean IsEnabled (GLenum cap);
	public static void EnableClientState (GLenum cap);
	public static void DisableClientState (GLenum cap);
	[CCode (array_length = false)]
	public static void GetBooleanv (GLenum pname, GLboolean[] params);
	[CCode (array_length = false)]
	public static void GetDoublev (GLenum pname, GLdouble[] params);
	[CCode (array_length = false)]
	public static void GetFloatv (GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void GetIntegerv (GLenum pname, GLint[] params);
	public static void PushAttrib (GLbitfield mask);
	public static void PopAttrib ();
	public static void PushClientAttrib (GLbitfield mask);
	public static void PopClientAttrib ();
	public static GLint RenderMode (GLenum mode);
	public static GLenum GetError ();
	public static weak string GetString (GLenum name); //original: GLubyte*
	public static void Finish ();
	public static void Flush ();
	public static void Hint (GLenum target, GLenum mode);

	// Depth Buffer
	public static void ClearDepth (GLclampd depth);
	public static void DepthFunc (GLenum func);
	public static void DepthMask (GLboolean flag);
	public static void DepthRange (GLclampd near_val, GLclampd far_val);

	// Accumulation Buffer
	public static void ClearAccum (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	public static void Accum (GLenum op, GLfloat @value);

	// Transformation
	public static void MatrixMode (GLenum mode);
	public static void Ortho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
	public static void Frustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
	public static void Viewport (GLint x, GLint y, GLsizei width, GLsizei height);
	public static void PushMatrix ();
	public static void PopMatrix ();
	public static void LoadIdentity ();
	[CCode (array_length = false)]
	public static void LoadMatrixd (GLdouble[] m);
	[CCode (array_length = false)]
	public static void LoadMatrixf (GLfloat[] m);
	[CCode (array_length = false)]
	public static void MultMatrixd (GLdouble[] m);
	[CCode (array_length = false)]
	public static void MultMatrixf (GLfloat[] m);
	public static void Rotated (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
	public static void Rotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	public static void Scaled (GLdouble x, GLdouble y, GLdouble z);
	public static void Scalef (GLfloat x, GLfloat y, GLfloat z);
	public static void Translated (GLdouble x, GLdouble y, GLdouble z);
	public static void Translatef (GLfloat x, GLfloat y, GLfloat z);

	// Display Lists
	public static GLboolean IsList (GLuint list);
	public static void DeleteLists (GLuint list, GLsizei range);
	public static GLuint GenLists (GLsizei range);
	public static void NewList (GLuint list, GLenum mode);
	public static void EndList ();
	public static void CallList (GLuint list);
	[CCode (array_length = false)]
	public static void CallLists (GLsizei n, GLenum type, GLvoid[] lists);
	public static void ListBase (GLuint @base);

	// Drawing Functions
	public static void Begin (GLenum mode);
	public static void End ();
	public static void Vertex2d (GLdouble x, GLdouble y);
	public static void Vertex2f (GLfloat x, GLfloat y);
	public static void Vertex2i (GLint x, GLint y);
	public static void Vertex2s (GLshort x, GLshort y);
	public static void Vertex3d (GLdouble x, GLdouble y, GLdouble z);
	public static void Vertex3f (GLfloat x, GLfloat y, GLfloat z);
	public static void Vertex3i (GLint x, GLint y, GLint z);
	public static void Vertex3s (GLshort x, GLshort y, GLshort z);
	public static void Vertex4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	public static void Vertex4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	public static void Vertex4i (GLint x, GLint y, GLint z, GLint w);
	public static void Vertex4s (GLshort x, GLshort y, GLshort z, GLshort w);
	[CCode (array_length = false)]
	public static void Vertex2dv (GLdouble[] v);
	[CCode (array_length = false)]
	public static void Vertex2fv (GLfloat[] v);
	[CCode (array_length = false)]
	public static void Vertex2iv (GLint[] v);
	[CCode (array_length = false)]
	public static void Vertex2sv (GLshort[] v);
	[CCode (array_length = false)]
	public static void Vertex3dv (GLdouble[] v);
	[CCode (array_length = false)]
	public static void Vertex3fv (GLfloat[] v);
	[CCode (array_length = false)]
	public static void Vertex3iv (GLint[] v);
	[CCode (array_length = false)]
	public static void Vertex3sv (GLshort[] v);
	[CCode (array_length = false)]
	public static void Vertex4dv (GLdouble[] v);
	[CCode (array_length = false)]
	public static void Vertex4fv (GLfloat[] v);
	[CCode (array_length = false)]
	public static void Vertex4iv (GLint[] v);
	[CCode (array_length = false)]
	public static void Vertex4sv (GLshort[] v);
	public static void Normal3b (GLbyte nx, GLbyte ny, GLbyte nz);
	public static void Normal3d (GLdouble nx, GLdouble ny, GLdouble nz);
	public static void Normal3f (GLfloat nx, GLfloat ny, GLfloat nz);
	public static void Normal3i (GLint nx, GLint ny, GLint nz);
	public static void Normal3s (GLshort nx, GLshort ny, GLshort nz);
	[CCode (array_length = false)]
	public static void Normal3bv (GLbyte[] v);
	[CCode (array_length = false)]
	public static void Normal3dv (GLdouble[] v);
	[CCode (array_length = false)]
	public static void Normal3fv (GLfloat[] v);
	[CCode (array_length = false)]
	public static void Normal3iv (GLint[] v);
	[CCode (array_length = false)]
	public static void Normal3sv (GLshort[] v);
	public static void Indexd (GLdouble c);
	public static void Indexf (GLfloat c);
	public static void Indexi (GLint c);
	public static void Indexs (GLshort c);
	public static void Indexub (GLubyte c);
	[CCode (array_length = false)]
	public static void Indexdv (GLdouble[] c);
	[CCode (array_length = false)]
	public static void Indexfv (GLfloat[] c);
	[CCode (array_length = false)]
	public static void Indexiv (GLint[] c);
	[CCode (array_length = false)]
	public static void Indexsv (GLshort[] c);
	[CCode (array_length = false)]
	public static void Indexubv (GLubyte[] c);
	public static void Color3b (GLbyte red, GLbyte green, GLbyte blue);
	public static void Color3d (GLdouble red, GLdouble green, GLdouble blue);
	public static void Color3f (GLfloat red, GLfloat green, GLfloat blue);
	public static void Color3i (GLint red, GLint green, GLint blue);
	public static void Color3s (GLshort red, GLshort green, GLshort blue);
	public static void Color3ub (GLubyte red, GLubyte green, GLubyte blue);
	public static void Color3ui (GLuint red, GLuint green, GLuint blue);
	public static void Color3us (GLushort red, GLushort green, GLushort blue);
	public static void Color4b (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
	public static void Color4d (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
	public static void Color4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	public static void Color4i (GLint red, GLint green, GLint blue, GLint alpha);
	public static void Color4s (GLshort red, GLshort green, GLshort blue, GLshort alpha);
	public static void Color4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
	public static void Color4ui (GLuint red, GLuint green, GLuint blue, GLuint alpha);
	public static void Color4us (GLushort red, GLushort green, GLushort blue, GLushort alpha);
	[CCode (array_length = false)]
	public static void Color3bv (GLbyte[] v);
	[CCode (array_length = false)]
	public static void Color3dv (GLdouble[] v);
	[CCode (array_length = false)]
	public static void Color3fv (GLfloat[] v);
	[CCode (array_length = false)]
	public static void Color3iv (GLint[] v);
	[CCode (array_length = false)]
	public static void Color3sv (GLshort[] v);
	[CCode (array_length = false)]
	public static void Color3ubv (GLubyte[] v);
	[CCode (array_length = false)]
	public static void Color3uiv (GLuint[] v);
	[CCode (array_length = false)]
	public static void Color3usv (GLushort[] v);
	[CCode (array_length = false)]
	public static void Color4bv (GLbyte[] v);
	[CCode (array_length = false)]
	public static void Color4dv (GLdouble[] v);
	[CCode (array_length = false)]
	public static void Color4fv (GLfloat[] v);
	[CCode (array_length = false)]
	public static void Color4iv (GLint[] v);
	[CCode (array_length = false)]
	public static void Color4sv (GLshort[] v);
	[CCode (array_length = false)]
	public static void Color4ubv (GLubyte[] v);
	[CCode (array_length = false)]
	public static void Color4uiv (GLuint[] v);
	[CCode (array_length = false)]
	public static void Color4usv (GLushort[] v);
	public static void TexCoord1d (GLdouble s);
	public static void TexCoord1f (GLfloat s);
	public static void TexCoord1i (GLint s);
	public static void TexCoord1s (GLshort s);
	public static void TexCoord2d (GLdouble s, GLdouble t);
	public static void TexCoord2f (GLfloat s, GLfloat t);
	public static void TexCoord2i (GLint s, GLint t);
	public static void TexCoord2s (GLshort s, GLshort t);
	public static void TexCoord3d (GLdouble s, GLdouble t, GLdouble r);
	public static void TexCoord3f (GLfloat s, GLfloat t, GLfloat r);
	public static void TexCoord3i (GLint s, GLint t, GLint r);
	public static void TexCoord3s (GLshort s, GLshort t, GLshort r);
	public static void TexCoord4d (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
	public static void TexCoord4f (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	public static void TexCoord4i (GLint s, GLint t, GLint r, GLint q);
	public static void TexCoord4s (GLshort s, GLshort t, GLshort r, GLshort q);
	[CCode (array_length = false)]
	public static void TexCoord1dv (GLdouble[] v);
	[CCode (array_length = false)]
	public static void TexCoord1fv (GLfloat[] v);
	[CCode (array_length = false)]
	public static void TexCoord1iv (GLint[] v);
	[CCode (array_length = false)]
	public static void TexCoord1sv (GLshort[] v);
	[CCode (array_length = false)]
	public static void TexCoord2dv (GLdouble[] v);
	[CCode (array_length = false)]
	public static void TexCoord2fv (GLfloat[] v);
	[CCode (array_length = false)]
	public static void TexCoord2iv (GLint[] v);
	[CCode (array_length = false)]
	public static void TexCoord2sv (GLshort[] v);
	[CCode (array_length = false)]
	public static void TexCoord3dv (GLdouble[] v);
	[CCode (array_length = false)]
	public static void TexCoord3fv (GLfloat[] v);
	[CCode (array_length = false)]
	public static void TexCoord3iv (GLint[] v);
	[CCode (array_length = false)]
	public static void TexCoord3sv (GLshort[] v);
	[CCode (array_length = false)]
	public static void TexCoord4dv (GLdouble[] v);
	[CCode (array_length = false)]
	public static void TexCoord4fv (GLfloat[] v);
	[CCode (array_length = false)]
	public static void TexCoord4iv (GLint[] v);
	[CCode (array_length = false)]
	public static void TexCoord4sv (GLshort[] v);
	public static void RasterPos2d (GLdouble x, GLdouble y);
	public static void RasterPos2f (GLfloat x, GLfloat y);
	public static void RasterPos2i (GLint x, GLint y);
	public static void RasterPos2s (GLshort x, GLshort y);
	public static void RasterPos3d (GLdouble x, GLdouble y, GLdouble z);
	public static void RasterPos3f (GLfloat x, GLfloat y, GLfloat z);
	public static void RasterPos3i (GLint x, GLint y, GLint z);
	public static void RasterPos3s (GLshort x, GLshort y, GLshort z);
	public static void RasterPos4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	public static void RasterPos4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	public static void RasterPos4i (GLint x, GLint y, GLint z, GLint w);
	public static void RasterPos4s (GLshort x, GLshort y, GLshort z, GLshort w);
	[CCode (array_length = false)]
	public static void RasterPos2dv (GLdouble[] v);
	[CCode (array_length = false)]
	public static void RasterPos2fv (GLfloat[] v);
	[CCode (array_length = false)]
	public static void RasterPos2iv (GLint[] v);
	[CCode (array_length = false)]
	public static void RasterPos2sv (GLshort[] v);
	[CCode (array_length = false)]
	public static void RasterPos3dv (GLdouble[] v);
	[CCode (array_length = false)]
	public static void RasterPos3fv (GLfloat[] v);
	[CCode (array_length = false)]
	public static void RasterPos3iv (GLint[] v);
	[CCode (array_length = false)]
	public static void RasterPos3sv (GLshort[] v);
	[CCode (array_length = false)]
	public static void RasterPos4dv (GLdouble[] v);
	[CCode (array_length = false)]
	public static void RasterPos4fv (GLfloat[] v);
	[CCode (array_length = false)]
	public static void RasterPos4iv (GLint[] v);
	[CCode (array_length = false)]
	public static void RasterPos4sv (GLshort[] v);
	public static void Rectd (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
	public static void Rectf (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
	public static void Recti (GLint x1, GLint y1, GLint x2, GLint y2);
	public static void Rects (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
	[CCode (array_length = false)]
	public static void Rectdv (GLdouble[] v1, GLdouble[] v2);
	[CCode (array_length = false)]
	public static void Rectfv (GLfloat[] v1, GLfloat[] v2);
	[CCode (array_length = false)]
	public static void Rectiv (GLint[] v1, GLint[] v2);
	[CCode (array_length = false)]
	public static void Rectsv (GLshort[] v1, GLshort[] v2);

	// Vertex Arrays  (1.1)
	public static void VertexPointer (GLint size, GLenum type, GLsizei stride, GLvoid* ptr);
	public static void NormalPointer (GLenum type, GLsizei stride, GLvoid* ptr);
	public static void ColorPointer (GLint size, GLenum type, GLsizei stride, GLvoid* ptr);
	public static void IndexPointer (GLenum type, GLsizei stride, GLvoid* ptr);
	public static void TexCoordPointer (GLint size, GLenum type, GLsizei stride, GLvoid* ptr);
	public static void EdgeFlagPointer (GLsizei stride, GLvoid* ptr);
	public static void GetPointerv (GLenum pname, GLvoid** params); 
	public static void ArrayElement (GLint i);
	public static void DrawArrays (GLenum mode, GLint first, GLsizei count);
	public static void DrawElements (GLenum mode, GLsizei count, GLenum type, GLvoid* indices);
	public static void InterleavedArrays (GLenum format, GLsizei stride, GLvoid* pointer);

	// Lighting
	public static void ShadeModel (GLenum mode);
	public static void Lightf (GLenum light, GLenum pname, GLfloat param);
	public static void Lighti (GLenum light, GLenum pname, GLint param);
	[CCode (array_length = false)]
	public static void Lightfv (GLenum light, GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void Lightiv (GLenum light, GLenum pname, GLint[] params);
	[CCode (array_length = false)]
	public static void GetLightfv (GLenum light, GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void GetLightiv (GLenum light, GLenum pname, GLint[] params);
	public static void LightModelf (GLenum pname, GLfloat param);
	public static void LightModeli (GLenum pname, GLint param);
	[CCode (array_length = false)]
	public static void LightModelfv (GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void LightModeliv (GLenum pname, GLint[] params);
	public static void Materialf (GLenum face, GLenum pname, GLfloat param);
	public static void Materiali (GLenum face, GLenum pname, GLint param);
	[CCode (array_length = false)]
	public static void Materialfv (GLenum face, GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void Materialiv (GLenum face, GLenum pname, GLint[] params);
	[CCode (array_length = false)]
	public static void GetMaterialfv (GLenum face, GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void GetMaterialiv (GLenum face, GLenum pname, GLint[] params);
	public static void ColorMaterial (GLenum face, GLenum mode);

	// Raster functions
	public static void PixelZoom (GLfloat xfactor, GLfloat yfactor);
	public static void PixelStoref (GLenum pname, GLfloat param);
	public static void PixelStorei (GLenum pname, GLint param);
	public static void PixelTransferf (GLenum pname, GLfloat param);
	public static void PixelTransferi (GLenum pname, GLint param);
	[CCode (array_length = false)]
	public static void PixelMapfv (GLenum map, GLsizei mapsize, GLfloat[] values);
	[CCode (array_length = false)]
	public static void PixelMapuiv (GLenum map, GLsizei mapsize, GLuint[] values);
	[CCode (array_length = false)]
	public static void PixelMapusv (GLenum map, GLsizei mapsize, GLushort[] values);
	[CCode (array_length = false)]
	public static void GetPixelMapfv (GLenum map, GLfloat[] values);
	[CCode (array_length = false)]
	public static void GetPixelMapuiv (GLenum map, GLuint[] values);
	[CCode (array_length = false)]
	public static void GetPixelMapusv (GLenum map, GLushort[] values);
	public static void Bitmap (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, GLubyte* bitmap);
	public static void ReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
	public static void DrawPixels (GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
	public static void CopyPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);

	// Stenciling
	public static void StencilFunc (GLenum func, GLint @ref, GLuint mask);
	public static void StencilMask (GLuint mask);
	public static void StencilOp (GLenum fail, GLenum zfail, GLenum zpass);
	public static void ClearStencil (GLint s);

	// Texture mapping
	public static void TexGend (GLenum coord, GLenum pname, GLdouble param);
	public static void TexGenf (GLenum coord, GLenum pname, GLfloat param);
	public static void TexGeni (GLenum coord, GLenum pname, GLint param);
	[CCode (array_length = false)]
	public static void TexGendv (GLenum coord, GLenum pname, GLdouble[] params);
	[CCode (array_length = false)]
	public static void TexGenfv (GLenum coord, GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void TexGeniv (GLenum coord, GLenum pname, GLint[] params);
	[CCode (array_length = false)]
	public static void GetTexGendv (GLenum coord, GLenum pname, GLdouble[] params);
	[CCode (array_length = false)]
	public static void GetTexGenfv (GLenum coord, GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void GetTexGeniv (GLenum coord, GLenum pname, GLint[] params);
	public static void TexEnvf (GLenum target, GLenum pname, GLfloat param);
	public static void TexEnvi (GLenum target, GLenum pname, GLint param);
	[CCode (array_length = false)]
	public static void TexEnvfv (GLenum target, GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void TexEnviv (GLenum target, GLenum pname, GLint[] params);
	[CCode (array_length = false)]
	public static void GetTexEnvfv (GLenum target, GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void GetTexEnviv (GLenum target, GLenum pname, GLint[] params);
	public static void TexParameterf (GLenum target, GLenum pname, GLfloat param);
	public static void TexParameteri (GLenum target, GLenum pname, GLint param);
	[CCode (array_length = false)]
	public static void TexParameterfv (GLenum target, GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void TexParameteriv (GLenum target, GLenum pname, GLint[] params);
	[CCode (array_length = false)]
	public static void GetTexParameterfv (GLenum target, GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void GetTexParameteriv (GLenum target, GLenum pname, GLint[] params);
	[CCode (array_length = false)]
	public static void GetTexLevelParameterfv (GLenum target, GLint level, GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void GetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint[] params);
	public static void TexImage1D (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, GLvoid* pixels);
	public static void TexImage2D (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLvoid* pixels);
	public static void GetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels);

	// 1.1 functions
	[CCode (array_length = false)]
	public static void GenTextures (GLsizei n, GLuint[] textures);
	[CCode (array_length = false)]
	public static void DeleteTextures (GLsizei n, GLuint[] textures);
	public static void BindTexture (GLenum target, GLuint texture);
	[CCode (array_length = false)]
	public static void PrioritizeTextures (GLsizei n, GLuint[] textures, GLclampf[] priorities);
	[CCode (array_length = false)]
	public static GLboolean AreTexturesResident (GLsizei n, GLuint[] textures, GLboolean[] residences);
	public static GLboolean IsTexture (GLuint texture);
	public static void TexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, GLvoid* pixels);
	public static void TexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
	public static void CopyTexImage1D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
	public static void CopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	public static void CopyTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	public static void CopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

	// Evaluators
	[CCode (array_length = false)]
	public static void Map1d (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, GLdouble[] points);
	[CCode (array_length = false)]
	public static void Map1f (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, GLfloat[] points);
	[CCode (array_length = false)]
	public static void Map2d (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, GLdouble[] points);
	[CCode (array_length = false)]
	public static void Map2f (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, GLfloat[] points);
	[CCode (array_length = false)]
	public static void GetMapdv (GLenum target, GLenum query, GLdouble[] v);
	[CCode (array_length = false)]
	public static void GetMapfv (GLenum target, GLenum query, GLfloat[] v);
	[CCode (array_length = false)]
	public static void GetMapiv (GLenum target, GLenum query, GLint[] v);
	public static void EvalCoord1d (GLdouble u);
	public static void EvalCoord1f (GLfloat u);
	[CCode (array_length = false)]
	public static void EvalCoord1dv (GLdouble[] u);
	[CCode (array_length = false)]
	public static void EvalCoord1fv (GLfloat[] u);
	public static void EvalCoord2d (GLdouble u, GLdouble v);
	public static void EvalCoord2f (GLfloat u, GLfloat v);
	[CCode (array_length = false)]
	public static void EvalCoord2dv (GLdouble[] u);
	[CCode (array_length = false)]
	public static void EvalCoord2fv (GLfloat[] u);
	public static void MapGrid1d (GLint un, GLdouble u1, GLdouble u2);
	public static void MapGrid1f (GLint un, GLfloat u1, GLfloat u2);
	public static void MapGrid2d (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
	public static void MapGrid2f (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
	public static void EvalPoint1 (GLint i);
	public static void EvalPoint2 (GLint i, GLint j);
	public static void EvalMesh1 (GLenum mode, GLint i1, GLint i2);
	public static void EvalMesh2 (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);

	// Fog
	public static void Fogf (GLenum pname, GLfloat param);
	public static void Fogi (GLenum pname, GLint param);
	[CCode (array_length = false)]
	public static void Fogfv (GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void Fogiv (GLenum pname, GLint[] params);

	// Selection and Feedback
	public static void FeedbackBuffer (GLsizei size, GLenum type, out GLfloat buffer);
	public static void PassThrough (GLfloat token);
	public static void SelectBuffer (GLsizei size, out GLuint buffer);
	public static void InitNames ();
	public static void LoadName (GLuint name);
	public static void PushName (GLuint name);
	public static void PopName ();
	
	// OpenGL 1.2
	public static void DrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, GLvoid* indices);
	public static void TexImage3D (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, GLvoid* pixels);
	public static void TexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLvoid* pixels);
	public static void CopyTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	
	// GL_ARB_imaging
	public static void ColorTable (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, GLvoid* table);
	public static void ColorSubTable (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, GLvoid* data);
	[CCode (array_length = false)]
	public static void ColorTableParameteriv (GLenum target, GLenum pname, GLint[] params);
	[CCode (array_length = false)]
	public static void ColorTableParameterfv (GLenum target, GLenum pname, GLfloat[] params);
	public static void CopyColorSubTable (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
	public static void CopyColorTable (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
	public static void GetColorTable (GLenum target, GLenum format, GLenum type, out GLvoid table);
	public static void GetColorTableParameterfv (GLenum target, GLenum pname, out GLfloat params);
	public static void GetColorTableParameteriv (GLenum target, GLenum pname, out GLint params);
	public static void BlendEquation (GLenum mode);
	public static void BlendColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	public static void Histogram (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
	public static void ResetHistogram (GLenum target);
	public static void GetHistogram (GLenum target, GLboolean reset, GLenum format, GLenum type, out GLvoid values);
	public static void GetHistogramParameterfv (GLenum target, GLenum pname, out GLfloat params);
	public static void GetHistogramParameteriv (GLenum target, GLenum pname, out GLint params);
	public static void Minmax (GLenum target, GLenum internalformat, GLboolean sink);
	public static void ResetMinmax (GLenum target);
	public static void GetMinmax (GLenum target, GLboolean reset, GLenum format, GLenum types, out GLvoid values);
	[CCode (array_length = false)]
	public static void GetMinmaxParameterfv (GLenum target, GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void GetMinmaxParameteriv (GLenum target, GLenum pname, GLint[] params);
	public static void ConvolutionFilter1D (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, GLvoid* image);
	public static void ConvolutionFilter2D (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* image);
	public static void ConvolutionParameterf (GLenum target, GLenum pname, GLfloat params);
	[CCode (array_length = false)]
	public static void ConvolutionParameterfv (GLenum target, GLenum pname, GLfloat[] params);
	public static void ConvolutionParameteri (GLenum target, GLenum pname, GLint params);
	[CCode (array_length = false)]
	public static void ConvolutionParameteriv (GLenum target, GLenum pname, GLint[] params);
	public static void CopyConvolutionFilter1D (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
	public static void CopyConvolutionFilter2D (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
	public static void GetConvolutionFilter (GLenum target, GLenum format, GLenum type, GLvoid *image);
	[CCode (array_length = false)]
	public static void GetConvolutionParameterfv (GLenum target, GLenum pname, GLfloat[] params);
	[CCode (array_length = false)]
	public static void GetConvolutionParameteriv (GLenum target, GLenum pname, GLint[] params);
	public static void SeparableFilter2D (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* row, GLvoid* column);
	public static void GetSeparableFilter (GLenum target, GLenum format, GLenum type, out GLvoid row, out GLvoid column, out GLvoid span);

	//OpenGL 1.3
	public static void ActiveTexture (GLenum texture);
	public static void ClientActiveTexture (GLenum texture);
	public static void CompressedTexImage1D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, GLvoid* data);
	public static void CompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLvoid* data);
	public static void CompressedTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, GLvoid* data);
	public static void CompressedTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, GLvoid* data);
	public static void CompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, GLvoid* data);
	public static void CompressedTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, GLvoid* data);
	public static void GetCompressedTexImage (GLenum target, GLint lod, out GLvoid img);
	public static void MultiTexCoord1d (GLenum target, GLdouble s);
	[CCode (array_length = false)]
	public static void MultiTexCoord1dv (GLenum target, GLdouble[] v);
	public static void MultiTexCoord1f (GLenum target, GLfloat s);
	[CCode (array_length = false)]
	public static void MultiTexCoord1fv (GLenum target, GLfloat[] v);
	public static void MultiTexCoord1i (GLenum target, GLint s);
	[CCode (array_length = false)]
	public static void MultiTexCoord1iv (GLenum target, GLint[] v);
	public static void MultiTexCoord1s (GLenum target, GLshort s);
	[CCode (array_length = false)]
	public static void MultiTexCoord1sv (GLenum target, GLshort[] v);
	public static void MultiTexCoord2d (GLenum target, GLdouble s, GLdouble t);
	[CCode (array_length = false)]
	public static void MultiTexCoord2dv (GLenum target, GLdouble[] v);
	public static void MultiTexCoord2f (GLenum target, GLfloat s, GLfloat t);
	[CCode (array_length = false)]
	public static void MultiTexCoord2fv (GLenum target, GLfloat[] v);
	public static void MultiTexCoord2i (GLenum target, GLint s, GLint t);
	[CCode (array_length = false)]
	public static void MultiTexCoord2iv (GLenum target, GLint[] v);
	public static void MultiTexCoord2s (GLenum target, GLshort s, GLshort t);
	[CCode (array_length = false)]
	public static void MultiTexCoord2sv (GLenum target, GLshort[] v);
	public static void MultiTexCoord3d (GLenum target, GLdouble s, GLdouble t, GLdouble r);
	[CCode (array_length = false)]
	public static void MultiTexCoord3dv (GLenum target, GLdouble[] v);
	public static void MultiTexCoord3f (GLenum target, GLfloat s, GLfloat t, GLfloat r);
	[CCode (array_length = false)]
	public static void MultiTexCoord3fv (GLenum target, GLfloat[] v);
	public static void MultiTexCoord3i (GLenum target, GLint s, GLint t, GLint r);
	[CCode (array_length = false)]
	public static void MultiTexCoord3iv (GLenum target, GLint[] v);
	public static void MultiTexCoord3s (GLenum target, GLshort s, GLshort t, GLshort r);
	[CCode (array_length = false)]
	public static void MultiTexCoord3sv (GLenum target, GLshort[] v);
	public static void MultiTexCoord4d (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
	[CCode (array_length = false)]
	public static void MultiTexCoord4dv (GLenum target, GLdouble[] v);
	public static void MultiTexCoord4f (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	[CCode (array_length = false)]
	public static void MultiTexCoord4fv (GLenum target, GLfloat[] v);
	public static void MultiTexCoord4i (GLenum target, GLint s, GLint t, GLint r, GLint q);
	[CCode (array_length = false)]
	public static void MultiTexCoord4iv (GLenum target, GLint[] v);
	public static void MultiTexCoord4s (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
	[CCode (array_length = false)]
	public static void MultiTexCoord4sv (GLenum target, GLshort[] v);
	[CCode (array_length = false)]
	public static void LoadTransposeMatrixd (GLdouble[16] m);
	[CCode (array_length = false)]
	public static void LoadTransposeMatrixf (GLfloat[16] m);
	[CCode (array_length = false)]
	public static void MultTransposeMatrixd (GLdouble[16] m);
	[CCode (array_length = false)]
	public static void MultTransposeMatrixf (GLfloat[16] m);
	public static void SampleCoverage (GLclampf @value, GLboolean invert);
	
	// GL_ARB_multitexture (ARB extension 1 and OpenGL 1.2.1)
	public static void ActiveTextureARB (GLenum texture);
	public static void ClientActiveTextureARB (GLenum texture);
	public static void MultiTexCoord1dARB (GLenum target, GLdouble s);
	[CCode (array_length = false)]
	public static void MultiTexCoord1dvARB (GLenum target, GLdouble[] v);
	public static void MultiTexCoord1fARB (GLenum target, GLfloat s);
	[CCode (array_length = false)]
	public static void MultiTexCoord1fvARB (GLenum target, GLfloat[] v);
	public static void MultiTexCoord1iARB (GLenum target, GLint s);
	[CCode (array_length = false)]
	public static void MultiTexCoord1ivARB (GLenum target, GLint[] v);
	public static void MultiTexCoord1sARB (GLenum target, GLshort s);
	[CCode (array_length = false)]
	public static void MultiTexCoord1svARB (GLenum target, GLshort[] v);
	public static void MultiTexCoord2dARB (GLenum target, GLdouble s, GLdouble t);
	[CCode (array_length = false)]
	public static void MultiTexCoord2dvARB (GLenum target, GLdouble[] v);
	public static void MultiTexCoord2fARB (GLenum target, GLfloat s, GLfloat t);
	[CCode (array_length = false)]
	public static void MultiTexCoord2fvARB (GLenum target, GLfloat[] v);
	public static void MultiTexCoord2iARB (GLenum target, GLint s, GLint t);
	[CCode (array_length = false)]
	public static void MultiTexCoord2ivARB (GLenum target, GLint[] v);
	public static void MultiTexCoord2sARB (GLenum target, GLshort s, GLshort t);
	[CCode (array_length = false)]
	public static void MultiTexCoord2svARB (GLenum target, GLshort[] v);
	public static void MultiTexCoord3dARB (GLenum target, GLdouble s, GLdouble t, GLdouble r);
	[CCode (array_length = false)]
	public static void MultiTexCoord3dvARB (GLenum target, GLdouble[] v);
	public static void MultiTexCoord3fARB (GLenum target, GLfloat s, GLfloat t, GLfloat r);
	[CCode (array_length = false)]
	public static void MultiTexCoord3fvARB (GLenum target, GLfloat[] v);
	public static void MultiTexCoord3iARB (GLenum target, GLint s, GLint t, GLint r);
	[CCode (array_length = false)]
	public static void MultiTexCoord3ivARB (GLenum target, GLint[] v);
	public static void MultiTexCoord3sARB (GLenum target, GLshort s, GLshort t, GLshort r);
	[CCode (array_length = false)]
	public static void MultiTexCoord3svARB (GLenum target, GLshort[] v);
	public static void MultiTexCoord4dARB (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
	[CCode (array_length = false)]
	public static void MultiTexCoord4dvARB (GLenum target, GLdouble[] v);
	public static void MultiTexCoord4fARB (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	[CCode (array_length = false)]
	public static void MultiTexCoord4fvARB (GLenum target, GLfloat[] v);
	public static void MultiTexCoord4iARB (GLenum target, GLint s, GLint t, GLint r, GLint q);
	[CCode (array_length = false)]
	public static void MultiTexCoord4ivARB (GLenum target, GLint[] v);
	public static void MultiTexCoord4sARB (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
	[CCode (array_length = false)]
	public static void MultiTexCoord4svARB (GLenum target, GLshort[] v);
  
  // OpenGL 1.4 (added by GGobi team)
  public static void BlendFuncSeparate (GLenum srcRGB, GLenum dstRGB,
                                        GLenum srcAlpha, GLenum dstAlpha);

  // OpenGL 1.5
  namespace Consts {
    public const GLenum COORD_REPLACE;
    public const GLenum POINT_SPRITE;
    public const GLenum POINT_SPRITE_COORD_ORIGIN;
    public const GLenum LOWER_LEFT;
  }
  public static void PointParameteri ( GLenum name, GLint param );
}

