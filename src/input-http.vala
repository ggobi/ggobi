using GLib;
using Gsf;
using Xml;

[CCode (cheader_filename = "gsf-helper.h")]
[CCode (cheader_filename = "input-http.h")]
public class GGobi.InputHTTP : Input
{
  public string url { get; set construct; }
  public string content_type { get; set construct; }
  
  private NanoHTTP ctx;
  
  private uchar[] buf;
  private ulong buf_size;
  
  public InputHTTP (construct string url) { }
  
  construct {
    set_name(url);
    string _content_type; /* properties are not variables - can't be out params */
    ctx = new NanoHTTP(url, out _content_type);
    if (ctx == null) /* no meaningful errors provided by nanohttp */
      set_size(-1);
    else {
      set_size(ctx.content_length());
      content_type = _content_type;
    }
  }
  
  override Input
  Dup(out GLib.Error err)
  { // need to fill in error here
    Input dup = new InputHTTP(url);
    return dup;
  }
  
  override weak uchar[]
  Read(ulong num_bytes, uchar[] buffer)
  {
    int nread;
    uint total_read;
    
    if (buffer == null) {
      if (buf_size < num_bytes) {
        buf_size = num_bytes;
        buf = new uchar[buf_size];
      }
      buffer = buf;
    }
  
    for (total_read = 0; total_read < num_bytes; total_read += nread) {
      nread = ctx.read((pointer)buffer, (int)(num_bytes - total_read));
      if (nread <= 0)
        return null;
    }
    return buffer;
  }
  
  override bool
  Seek(int64 offset, SeekType whence)
  {
    return false;
  }
  
}
