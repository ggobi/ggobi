using GLib;
using Gsf;
using Xml;

/* this trick lets us include arbitrary C code at the top of the file */
[CCode (cheader_filename = "gsf-helper.h")]
[CCode (cheader_filename = "input-ftp.h")]
public class GGobi.InputFTP : Input
{
  public string url { get; set construct; }
   
  private NanoFTP ctx;
  private uchar[] buf;
  private ulong buf_size;
  
  // VALABUG: before, I didn't have an empty body, just a semicolon, but
  // vala did not catch the problem - the linking failed
  public InputFTP (construct string url) { }
  
  construct {
    set_name(url);
    string filename = Path.get_basename(url);
    ctx = new NanoFTP (url);
    if (ctx == null)
      set_size(-1); // indicates an error
    else ctx.list(
      (self, fn, attr, owner, grp, size, links, year, month, day, hr, min) 
        => set_size(size),
      null, filename);
  }
  
  
  override Input
  Dup(out GLib.Error err)
  { // need to fill in error here
    Input dup = new InputFTP(url);
    return dup;
  }
  
  override weak uchar[]
  Read(ulong num_bytes, uchar[] buffer)
  {
    int nread;
    uint total_read;
    
    if (ctx.get_connection() < 0)
      return null;
      
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
