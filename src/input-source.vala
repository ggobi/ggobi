using GLib;
using Gsf;

public abstract class GGobi.InputSource : Object {
  public string uri { get; set construct; }
  public string mode { get; set construct; }
  public string display_name { get; set construct; }

  /**
   * get_input:
   * @self: a #GGobiInputSource
   * @error: if not %NULL, set to any error encoutered when retrieving the data
   *
   * Gets the input stream of data.
   *
   * Returns: #GsfInput or %NULL if this input source cannot provide an input stream
   */
  public abstract Input
  get_input(out GLib.Error err);
}
