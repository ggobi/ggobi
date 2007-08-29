namespace Gsf {
	[CCode (cprefix = "GSF_CLIP_FORMAT_")]
	public enum ClipFormat {
		WINDOWS_CLIPBOARD,
		MACINTOSH_CLIPBOARD,
		GUID,
		NO_DATA,
		CLIPBOARD_FORMAT_NAME,
		UNKNOWN,
	}
	[CCode (cprefix = "GSF_CLIP_FORMAT_WINDOWS_")]
	public enum ClipFormatWindows {
		ERROR,
		UNKNOWN,
		METAFILE,
		DIB,
		ENHANCED_METAFILE,
	}
	[CCode (cprefix = "GSF_ERROR_")]
	public enum Error {
		OUT_OF_MEMORY,
		INVALID_DATA,
	}
	[CCode (cprefix = "GSF_OUTPUT_CSV_QUOTING_MODE_")]
	public enum OutputCsvQuotingMode {
		NEVER,
		AUTO,
		ALWAYS,
	}
	[CCode (cprefix = "GSF_XML_")]
	public enum XMLContent {
		NO_CONTENT,
		CONTENT,
		SHARED_CONTENT,
	}
	[CCode (cprefix = "GSF_ZIP_")]
	public enum ZipCompressionMethod {
		STORED,
		SHRUNK,
		REDUCEDx1,
		REDUCEDx2,
		REDUCEDx3,
		REDUCEDx4,
		IMPLODED,
		TOKENIZED,
		DEFLATED,
		DEFLATED_BETTER,
		IMPLODED_BETTER,
	}
	[CCode (cheader_filename = "gsf/gsf-blob.h")]
	public class Blob : GLib.Object {
		public ulong get_size ();
		public static GLib.Type get_type ();
		public Blob (ulong size, pointer data_to_copy, out GLib.Error error);
		public pointer peek_data ();
	}
	[CCode (cheader_filename = "gsf/gsf-docprop-vector.h")]
	public class DocPropVector : GLib.Object {
		[CCode (cname = "gsf_docprop_vector_append")]
		public void append (GLib.Value value);
		[CCode (cname = "gsf_docprop_vector_as_string")]
		public string as_string ();
		[CCode (cname = "gsf_docprop_vector_get_type")]
		public static GLib.Type get_type ();
		[CCode (cname = "gsf_docprop_vector_new")]
		public DocPropVector ();
	}
	[CCode (cheader_filename = "gsf/gsf-doc-meta-data.h")]
	public class DocMetaData : GLib.Object {
		public void @foreach (GLib.HFunc func, pointer user_data);
		public static GLib.Type get_type ();
		public void insert (string name, GLib.Value value);
		public weak Gsf.DocProp lookup (string name);
		public DocMetaData ();
		public void remove (string name);
		public ulong size ();
		public weak Gsf.DocProp steal (string name);
		public void store (Gsf.DocProp prop);
	}
	[CCode (cheader_filename = "gsf/gsf-infile-msole.h")]
	public class InfileMSOle : Gsf.Infile {
		[CCode (cname = "gsf_infile_msole_get_class_id")]
		public bool get_class_id (uchar res);
		[CCode (cname = "gsf_infile_msole_get_type")]
		public static GLib.Type get_type ();
		[CCode (cname = "gsf_infile_msole_new")]
		public InfileMSOle (Gsf.Input source, out GLib.Error err);
	}
	[CCode (cheader_filename = "gsf/gsf-infile-msvba.h")]
	public class InfileMSVBA : Gsf.Infile {
		[CCode (cname = "gsf_infile_msvba_get_type")]
		public static GLib.Type get_type ();
		[CCode (cname = "gsf_infile_msvba_new")]
		public InfileMSVBA (Gsf.Infile source, out GLib.Error err);
	}
	[CCode (cheader_filename = "gsf/gsf-infile-stdio.h")]
	public class InfileStdio : Gsf.Infile {
		public static GLib.Type get_type ();
		public InfileStdio (string root, out GLib.Error err);
	}
	[CCode (cheader_filename = "gsf/gsf-infile.h")]
	public class Infile : Gsf.Input {
		public weak Gsf.Input child_by_index (int i);
		public weak Gsf.Input child_by_name (string name);
		public weak Gsf.Input child_by_vname (string name);
		public static GLib.Type get_type ();
		public weak string name_by_index (int i);
		public int num_children ();
	}
	[CCode (cheader_filename = "gsf/gsf-infile-zip.h")]
	public class InfileZip : Gsf.Infile {
		public static GLib.Type get_type ();
		public InfileZip (Gsf.Input source, out GLib.Error err);
		[NoAccessorMethod]
		public weak Gsf.Input source { get; construct; }
		[NoAccessorMethod]
		public weak int compression_level { get; }
		[NoAccessorMethod]
		public weak Gsf.InfileZip internal_parent { construct; }
	}
	[CCode (cheader_filename = "gsf/gsf-input-gzip.h")]
	public class InputGZip : Gsf.Input {
		public static GLib.Type get_type ();
		public InputGZip (Gsf.Input source, out GLib.Error err);
		[NoAccessorMethod]
		public weak bool raw { get; construct; }
		[NoAccessorMethod]
		public weak Gsf.Input source { get; construct; }
		[NoAccessorMethod]
		public weak int64 uncompressed_size { get; construct; }
	}
	[CCode (cheader_filename = "gsf/gsf-input-memory.h")]
	public class InputMemory : Gsf.Input {
		public static GLib.Type get_type ();
		[NoArrayLength]
		public InputMemory (uchar[] buf, int64 length, bool needs_free);
		[NoArrayLength]
		public InputMemory.clone (uchar[] buf, int64 length);
		[CCode (cname = "gsf_input_memory_mmap_new")]
		public InputMemory.mmap (string filename, out GLib.Error err);
		public InputMemory.from_bzip (Gsf.Input source, out GLib.Error err);
		public InputMemory.from_iochannel (GLib.IOChannel channel, out GLib.Error error);
	}
	[CCode (cheader_filename = "gsf/gsf-input-proxy.h")]
	public class InputProxy : Gsf.Input {
		public static GLib.Type get_type ();
		public InputProxy (Gsf.Input source);
		public InputProxy.section (Gsf.Input source, int64 offset, int64 size);
	}
	[CCode (cheader_filename = "gsf/gsf-input-stdio.h")]
	public class InputStdio : Gsf.Input {
		public static GLib.Type get_type ();
		public InputStdio (string filename, out GLib.Error err);
	}
	[CCode (cheader_filename = "gsf/gsf-input-textline.h")]
	public class InputTextline : Gsf.Input {
		public static GLib.Type get_type ();
		public InputTextline (Gsf.Input source);
    [NoArrayLength]
		public weak uchar[] utf8_gets ();
	}
	[CCode (cheader_filename = "gsf/gsf-input-impl.h")]
	public class Input : GLib.Object {
		[CCode (cname = "Dup")]
		public virtual Gsf.Input Dup (out GLib.Error err);
		[CCode (cname = "Read")]
    [NoArrayLength]
		public virtual weak uchar[] Read (ulong num_bytes, uchar[] optional_buffer);
		[CCode (cname = "Seek")]
		public virtual bool Seek (int64 offset, GLib.SeekType whence);
		[CCode (cname = "OpenSibling")]
		public virtual Gsf.Input OpenSibling (string path, out GLib.Error err);
		public weak Gsf.Infile container ();
		public bool copy (Gsf.Output output);
		public void dump (bool dump_as_hex);
		public Gsf.Input dup (out GLib.Error err);
		public static GLib.Quark error_id ();
		public static GLib.Type get_type ();
		[NoArrayLength]
		public weak uchar[] read (ulong num_bytes, uchar[] optional_buffer);
		public bool seek (int64 offset, GLib.SeekType whence);
		public bool seek_emulate (int64 pos);
		public bool set_container (Gsf.Infile container);
		public bool set_name (string name);
		public bool set_name_from_filename (string filename);
		public bool set_size (int64 size);
		public Gsf.Input sibling (string name, out GLib.Error err);
		public int64 tell ();
		public Gsf.Input uncompress ();
		[NoAccessorMethod]
		public weak string name { get; }
		[NoAccessorMethod]
		public weak int64 size { get; }
		[NoAccessorMethod]
		public weak bool eof { get; }
		[NoAccessorMethod]
		public weak int64 remaining { get; }
		[NoAccessorMethod]
		public weak int64 position { get; }
	}
	[CCode (cheader_filename = "gsf/gsf-outfile-msole.h")]
	public class OutfileMSOle : Gsf.Outfile {
		[CCode (cname = "gsf_outfile_msole_get_type")]
		public static GLib.Type get_type ();
		[CCode (cname = "gsf_outfile_msole_new")]
		public OutfileMSOle (Gsf.Output sink);
		[CCode (cname = "gsf_outfile_msole_new_full")]
		public OutfileMSOle.full (Gsf.Output sink, uint bb_size, uint sb_size);
		[NoArrayLength]
		[CCode (cname = "gsf_outfile_msole_set_class_id")]
		public bool set_class_id (uchar[] clsid);
	}
	[CCode (cheader_filename = "gsf/gsf-outfile-stdio.h")]
	public class OutfileStdio : Gsf.Outfile {
		public static GLib.Type get_type ();
		public OutfileStdio (string root, out GLib.Error err);
		public OutfileStdio.full (string root, out GLib.Error err, string first_property_name);
		public OutfileStdio.valist (string root, out GLib.Error err, string first_property_name, pointer var_args);
	}
	[CCode (cheader_filename = "gsf/gsf-outfile-impl.h")]
	public class Outfile : Gsf.Output {
		public static GLib.Type get_type ();
		public Outfile.child (string name, bool is_dir);
		public Outfile.child_full (string name, bool is_dir, string first_property_name);
		public Outfile.child_varg (string name, bool is_dir, string first_property_name, pointer args);
	}
	[CCode (cheader_filename = "gsf/gsf-outfile-zip.h")]
	public class OutfileZip : Gsf.Outfile {
		public static GLib.Type get_type ();
		public OutfileZip (Gsf.Output sink, out GLib.Error err);
		public bool set_compression_method (Gsf.ZipCompressionMethod method);
		[NoAccessorMethod]
		public weak Gsf.Output sink { get; construct; }
		[NoAccessorMethod]
		public weak string entry_name { get; construct; }
		[NoAccessorMethod]
		public weak int compression_level { get; construct; }
	}
	[CCode (cheader_filename = "gsf/gsf-output-bzip.h")]
	public class OutputBzip : Gsf.Output {
		public static GLib.Type get_type ();
		public OutputBzip (Gsf.Output sink, out GLib.Error err);
	}
	[CCode (cheader_filename = "gsf/gsf-output-csv.h")]
	public class OutputCsv : Gsf.Output {
		public static GLib.Type get_type ();
		public bool write_eol ();
		public bool write_field (string field, ulong len);
		[NoAccessorMethod]
		public weak Gsf.Output sink { get; set; }
		[NoAccessorMethod]
		public weak string quote { get; set construct; }
		[NoAccessorMethod]
		public weak Gsf.OutputCsvQuotingMode quoting_mode { get; set construct; }
		[NoAccessorMethod]
		public weak string quoting_triggers { get; set; }
		[NoAccessorMethod]
		public weak string separator { get; set construct; }
		[NoAccessorMethod]
		public weak string eol { get; set construct; }
	}
	[CCode (cheader_filename = "gsf/gsf-output-gzip.h")]
	public class OutputGZip : Gsf.Output {
		public static GLib.Type get_type ();
		public OutputGZip (Gsf.Output sink, out GLib.Error err);
		[NoAccessorMethod]
		public weak bool raw { get; construct; }
		[NoAccessorMethod]
		public weak Gsf.Output sink { get; construct; }
	}
	[CCode (cheader_filename = "gsf/gsf-output-iconv.h")]
	public class OutputIconv : Gsf.Output {
		public static GLib.Type get_type ();
		public OutputIconv (Gsf.Output sink, string dst, string src);
		[NoAccessorMethod]
		public weak Gsf.Output sink { get; construct; }
		[NoAccessorMethod]
		public weak string input_charset { get; construct; }
		[NoAccessorMethod]
		public weak string output_charset { get; construct; }
		[NoAccessorMethod]
		public weak string fallback { get; set; }
	}
	[CCode (cheader_filename = "gsf/gsf-output-iochannel.h")]
	public class OutputIOChannel : Gsf.Output {
		[CCode (cname = "gsf_output_iochannel_get_type")]
		public static GLib.Type get_type ();
		[CCode (cname = "gsf_output_iochannel_new")]
		public OutputIOChannel (GLib.IOChannel channel);
	}
	[CCode (cheader_filename = "gsf/gsf-output-memory.h")]
	public class OutputMemory : Gsf.Output {
		[NoArrayLength]
		public weak uchar[] get_bytes ();
		public static GLib.Type get_type ();
		public OutputMemory ();
	}
	[CCode (cheader_filename = "gsf/gsf-output-stdio.h")]
	public class OutputStdio : Gsf.Output {
		public static GLib.Type get_type ();
		public OutputStdio (string filename, out GLib.Error err);
		public OutputStdio.full (string filename, out GLib.Error err, string first_property_name, ...);
	}
	[CCode (cheader_filename = "gsf/gsf-output-impl.h")]
	public class Output : GLib.Object {
		[CCode (cname = "Close")]
		public virtual bool Close ();
		[CCode (cname = "Seek")]
		public virtual bool Seek (int64 offset, GLib.SeekType whence);
		[NoArrayLength]
		[CCode (cname = "Write")]
		public virtual bool Write (uint num_bytes, uchar[] data);
		public bool close ();
		public weak Gsf.Outfile container ();
		public weak GLib.Error error ();
		public static GLib.Quark error_id ();
		public static GLib.Type get_type ();
		public bool printf (string format, ...);
		public bool puts (string line);
		public bool seek (int64 offset, GLib.SeekType whence);
		public bool set_container (Gsf.Outfile container);
		public bool set_error (int code, string format);
		public bool set_name (string name);
		public bool set_name_from_filename (string filename);
		public int64 tell ();
		public static bool unwrap (GLib.Object wrapper, Gsf.Output wrapee);
		public static bool wrap (GLib.Object wrapper, Gsf.Output wrapee);
		[NoArrayLength]
		public bool write (ulong num_bytes, uchar[] data);
		[NoAccessorMethod]
		public weak string name { get; }
		[NoAccessorMethod]
		public weak int64 size { get; }
		[NoAccessorMethod]
		public weak int64 position { get; }
		[NoAccessorMethod]
		public weak bool is_closed { get; }
	}
	[CCode (cheader_filename = "gsf/gsf-structured-blob.h")]
	public class StructuredBlob : Gsf.Infile {
		public static GLib.Type get_type ();
		public static Gsf.StructuredBlob read (Gsf.Input input);
		public bool write (Gsf.Outfile container);
	}
	[CCode (cheader_filename = "gsf/gsf-clip-data.h")]
	public class ClipData : GLib.Object {
		public weak Gsf.Blob get_data_blob ();
		public Gsf.ClipFormat get_format ();
		public static GLib.Type get_type ();
		public Gsf.ClipFormatWindows get_windows_clipboard_format (out GLib.Error error);
		public ClipData (Gsf.ClipFormat format, Gsf.Blob data_blob);
		public pointer peek_real_data (ulong ret_size, out GLib.Error error);
	}
	[CCode (cheader_filename = "gsf/gsf-libxml.h")]
	public class XMLOut : GLib.Object {
		[NoArrayLength]
		[CCode (cname = "gsf_xml_out_add_base64")]
		public void add_base64 (string id, uchar[] data, uint len);
		[CCode (cname = "gsf_xml_out_add_bool")]
		public void add_bool (string id, bool val);
		[CCode (cname = "gsf_xml_out_add_color")]
		public void add_color (string id, uint r, uint g, uint b);
		[CCode (cname = "gsf_xml_out_add_cstr")]
		public void add_cstr (string id, string val_utf8);
		[CCode (cname = "gsf_xml_out_add_cstr_unchecked")]
		public void add_cstr_unchecked (string id, string val_utf8);
		[CCode (cname = "gsf_xml_out_add_enum")]
		public void add_enum (string id, GLib.Type etype, int val);
		[CCode (cname = "gsf_xml_out_add_float")]
		public void add_float (string id, double val, int precision);
		[CCode (cname = "gsf_xml_out_add_gvalue")]
		public void add_gvalue (string id, GLib.Value val);
		[CCode (cname = "gsf_xml_out_add_int")]
		public void add_int (string id, int val);
		[CCode (cname = "gsf_xml_out_add_uint")]
		public void add_uint (string id, uint val);
		[CCode (cname = "gsf_xml_out_end_element")]
		public weak string end_element ();
		[CCode (cname = "gsf_xml_out_get_type")]
		public static GLib.Type get_type ();
		[CCode (cname = "gsf_xml_out_new")]
		public XMLOut (Gsf.Output output);
		[CCode (cname = "gsf_xml_out_set_doc_type")]
		public void set_doc_type (string type);
		[CCode (cname = "gsf_xml_out_simple_element")]
		public void simple_element (string id, string content);
		[CCode (cname = "gsf_xml_out_simple_float_element")]
		public void simple_float_element (string id, double val, int precision);
		[CCode (cname = "gsf_xml_out_simple_int_element")]
		public void simple_int_element (string id, int val);
		[CCode (cname = "gsf_xml_out_start_element")]
		public void start_element (string id);
		[NoAccessorMethod]
		public weak bool pretty_print { get; set; }
	}
	[ReferenceType]
	public struct DocProp {
		public void dump ();
		public void free ();
		public weak string get_link ();
		public weak string get_name ();
		public weak GLib.Value get_val ();
		public DocProp (string name);
		public void set_link (string link);
		public void set_val (GLib.Value val);
		public weak GLib.Value swap_val (GLib.Value val);
	}
	[ReferenceType]
	public struct Timestamp {
		public weak GLib.Date date;
		public long seconds;
		public weak GLib.String time_zone;
		public uint timet;
		public string as_string ();
		public Gsf.Timestamp copy ();
		public bool equal (Gsf.Timestamp b);
		public void free ();
		public static GLib.Type get_type ();
		public uint hash ();
		public static int parse (string spec, Gsf.Timestamp stamp);
	}
	[ReferenceType]
	public struct XMLBlob {
	}
	[ReferenceType]
	public struct XMLIn {
		public pointer user_state;
		public weak GLib.String content;
		public weak Gsf.XMLInDoc doc;
		public weak Gsf.XMLInNode node;
		public weak GLib.SList node_stack;
		[CCode (cname = "gsf_xml_in_check_ns")]
		public weak string check_ns (string str, uint ns_id);
		[CCode (cname = "gsf_xml_in_get_input")]
		public weak Gsf.Input get_input ();
		[CCode (cname = "gsf_xml_in_namecmp")]
		public bool namecmp (string str, uint ns_id, string name);
		[CCode (cname = "gsf_xml_in_push_state")]
		public void push_state (Gsf.XMLInDoc doc, pointer new_state, Gsf.XMLInExtDtor dtor, string attrs);
	}
	[ReferenceType]
	public struct XMLInDoc {
		[CCode (cname = "gsf_xml_in_doc_free")]
		public void free ();
		[NoArrayLength]
		[CCode (cname = "gsf_xml_in_doc_new")]
		public XMLInDoc (Gsf.XMLInNode[] nodes, Gsf.XMLInNS[] ns);
		[CCode (cname = "gsf_xml_in_doc_parse")]
		public bool parse (Gsf.Input input, pointer user_state);
		[CCode (cname = "gsf_xml_in_doc_set_unknown_handler")]
		public void set_unknown_handler (Gsf.XMLInUnknownFunc handler);
	}
	[ReferenceType]
	public struct XMLInNS {
		public weak string uri;
		public uint ns_id;
	}
  public delegate void XMLInNodeStart (XMLInNode state, string[] attrs);
  public delegate void XMLInNodeEnd (XMLInNode state, XMLBlob unknown);
	[ReferenceType]
	public struct XMLInNode {
		public weak string id;
		public int ns_id;
		public weak string name;
		public weak string parent_id;
		public weak pointer user_data;
    public XMLInNodeStart start;
    public XMLInNodeEnd end;
		public Gsf.XMLContent has_content;
		public uint check_children_for_ns;
		public uint share_children_with_parent;
	}
	[ReferenceType]
	public struct ZipDirent {
		public weak string name;
		public Gsf.ZipCompressionMethod compr_method;
		public uint crc32;
		public ulong csize;
		public ulong usize;
		public int64 offset;
		public int64 data_offset;
		public uint dostime;
		public void free ();
		public ZipDirent ();
	}
	[ReferenceType]
	public struct ZipVDir {
		public weak string name;
		public bool is_directory;
		public weak Gsf.ZipDirent dirent;
		public weak GLib.SList children;
		public weak GLib.SList last_child;
	}
	[ReferenceType]
	public struct Le {
		public static double get_double (pointer p);
		public static float get_float (pointer p);
		public static uint64 get_guint64 (pointer p);
		public static void set_double (pointer p, double d);
		public static void set_float (pointer p, float f);
	}
	[ReferenceType]
	public struct Msole {
		public static uint codepage_to_lid (int codepage);
		public static GLib.IConv iconv_open_codepage_for_export (int codepage_to);
		public static GLib.IConv iconv_open_codepage_for_import (string to, int codepage);
		public static GLib.IConv iconv_open_codepages_for_export (int codepage_to, string from);
		public static GLib.IConv iconv_open_for_export ();
		public static GLib.IConv iconv_open_for_import (int codepage);
		public static int iconv_win_codepage ();
		public static GLib.ByteArray inflate (Gsf.Input input, int64 offset);
		public static weak string language_for_lid (uint lid);
		public static uint lid_for_language (string lang);
		public static int lid_to_codepage (uint lid);
		public static weak string lid_to_codepage_str (uint lid);
		public static GLib.Error metadata_read (Gsf.Input @in, Gsf.DocMetaData accum);
		public static bool metadata_write (Gsf.Output @out, Gsf.DocMetaData meta_data, bool doc_not_component);
	}
	[ReferenceType]
	public struct Opendoc {
		public static GLib.Error metadata_read (Gsf.Input input, Gsf.DocMetaData md);
		public static void metadata_subtree (Gsf.XMLIn doc, Gsf.DocMetaData md);
		public static bool metadata_write (Gsf.XMLOut output, Gsf.DocMetaData md);
	}
	[ReferenceType]
	public struct Vdir {
		public static void add_child (Gsf.ZipVDir vdir, Gsf.ZipVDir child);
		public static void free (Gsf.ZipVDir vdir, bool free_dirent);
		public Vdir (string name, bool is_directory, Gsf.ZipDirent dirent);
	}
	[ReferenceType]
	public struct Xml {
	}
	public delegate void XMLInExtDtor (Gsf.XMLIn xin, pointer old_state);
	public delegate bool XMLInUnknownFunc (Gsf.XMLIn xin, string elem, string attrs);
	public static void doc_meta_dump (Gsf.DocMetaData meta);
	public static string filename_to_utf8 (string filename, bool quoted);
	public static void iconv_close (GLib.IConv handle);
	[NoArrayLength]
	public static void mem_dump (uchar[] ptr, ulong len);
}
