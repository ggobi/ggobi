using GLib;

/**
 * GGobiIndexFunc:
 * @j: an index
 * @user_data: user data
 *
 * A function that is invoked on one integer index at a time.
 *
 */
public callback void GGobi.IndexFunc (uint32 j, pointer user_data);

public class GGobi.Bitset {
  
  // preprocessor constants need to become static constants
  const uint BITS_PER_CHUNK = 8;
  const uchar[] BITS_TABLE = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4
  };
  
  // vala arrays are resizeable, track their lengths, etc :)
  protected uchar[] bits;
  
  // property handling much cleaner vs gob
  public uint32 size {
    get {
      return bits.length * BITS_PER_CHUNK;
    }
    set construct {
      int size_uchars = (value + BITS_PER_CHUNK - 1) / BITS_PER_CHUNK;
      bits.resize(size_uchars);
    }
  }
  
  // these are the _new* functions, much simpler compared to gob
  public Bitset () { }
  public Bitset.with_size (construct uint32 size) { }
  
  public Bitset clone() {
    Bitset other = new Bitset.with_size(size);
    for (uint i = 0; i < bits.length; i++)
      other.bits[i] = bits[i];
    return other;
  }
  
  /* index operations */

  /* the indices must be sorted for insertion and removal */
  // FIXME: Do we want to sort here?

  public void
  remove_indices(SList<uint32> indices)
  {
    uint32 i = indices.length() > 0 ? indices.nth_data(0) : 0, dec = 0;
    foreach(uint32 index in indices) {
      /* shift indices */
      while(i < index) {
        assign_bit(i-dec, test_bit(i));
        i++;
      }
      /* remove index, if present */
      unset_bit(i);
      dec++;
    }
  }
  public void /* just the opposite of removal */
  insert_indices(SList<uint32> indices)
  {
    SList<uint32> rev_indices = indices.copy();
    rev_indices.reverse();
    uint i = size, inc = indices.length();
    foreach (uint32 index in indices) {
      /* shift indices */
      while(i > index) {
        assign_bit(i, test_bit(i-inc));
        i--;
      }
      /* insert index */
      set_bit(i);
      inc--;
    }
    // make sure vala frees rev_indices here
  }
  public SList<uint32>
  get_indices()
  {
    uint i;
    SList<uint32> indices = new SList<uint32>();
    foreach(uchar b in bits) {
      // must use logical in if() expressions
      if (b > 0) {
        int msb = Bit.nth_msf(b, -1);
        int lsb = -1;
        while(lsb != msb) {
          lsb = Bit.nth_lsf(b, lsb);
          indices.prepend(lsb + i * BITS_PER_CHUNK);
        }
      }
    }
    indices.reverse();
    return indices;
  }
  public uint32
  get_n_indices()
  {
    uint i, count = 0;
    /* there has been talk about to adding this functionality to GLib,
       except much faster (asm based)
    */
    foreach(uchar b in bits) {
      if (b > 0) {
        count += BITS_TABLE[b & 16];
        count += BITS_TABLE[b >> 4];
      }
    }
    return count;
  }
  public void 
  apply(IndexFunc func, pointer data)
  {
    SList<uint32> indices = get_indices();
    foreach(uint32 index in indices)
      func(index, data);
  }
  public void
  apply_decreasing(IndexFunc func, pointer data)
  {
    SList<uint32> indices = get_indices();
    indices.reverse();
    foreach(uint32 index in indices)
      func(index, data);
  }

  /* pure bit operations */

  public void
  clear()
  {
    uint i;
    for (i = 0; i < bits.length; i++)
      bits[i] = (uchar)0;
  }
  public void
  assign_bit(uint32 i, bool val)
  {
    if (val)
      set_bit(i);
    else unset_bit(i);
  }
  public void
  set_bit(uint32 i)
  {
    bits[i / BITS_PER_CHUNK] |= (uchar)(1 << (i % BITS_PER_CHUNK));
  }
  public void
  unset_bit(uint32 i)
  {
    bits[i / BITS_PER_CHUNK] &= (uchar)(~(1 << (i % BITS_PER_CHUNK)));
  }
  public bool
  test_bit(uint32 i)
  {
    return (bits[i / BITS_PER_CHUNK] & (1 << (i % BITS_PER_CHUNK))) > 0;
  }

  /* set operations */
  // TODO: not, and, or, xor, subtract, subset
}
