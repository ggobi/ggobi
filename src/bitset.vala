using GLib;

/**
 * IndexFunc:
 * @j: an index
 * @user_data: user data
 *
 * A function that is invoked on one integer index at a time.
 *
 */
public delegate void GGobi.IndexFunc (uint j, pointer user_data);

public class GGobi.Bitset {
  
  const uint BITS_PER_CHUNK = 8;
  const uchar[] BITS_TABLE = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4
  };

  protected uchar[] bits;
  
  public uint size {
    get {
      return bits.length * BITS_PER_CHUNK;
    }
    set construct {
      int size_uchars = (value + BITS_PER_CHUNK - 1) / BITS_PER_CHUNK;
      bits.resize(size_uchars);
    }
  }
  
  public Bitset () { }
  public Bitset.with_size (construct uint size) { }
  
  public virtual Bitset clone() {
    Bitset other = new Bitset.with_size(size);
    for (uint i = 0; i < bits.length; i++)
      other.bits[i] = bits[i];
    return other;
  }
  
  /* index operations */

  /* the indices must be sorted for insertion and removal */
  // FIXME: Do we want to sort here?
  public void
  remove_indices(SList<uint> indices)
  {
    uint i = indices.length() > 0 ? indices.nth_data(0) : 0, dec = 0;
    foreach(uint index in indices) {
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
  insert_indices(SList<uint> indices)
  {
    SList<uint> rev_indices = indices.copy();
    rev_indices.reverse();
    uint i = size, inc = indices.length();
    foreach (uint index in indices) {
      /* shift indices */
      while(i > index) {
        assign_bit(i, test_bit(i-inc));
        i--;
      }
      /* insert index */
      set_bit(i);
      inc--;
    }
  }
  public SList<uint>
  get_indices()
  {
    uint i;
    SList<uint> indices = new SList<uint>();
    foreach(uchar b in bits) {
      if (b > 0) {
        int msb = Bit.nth_msf(b, -1);
        int lsb = -1;
        while(lsb != msb) {
          lsb = Bit.nth_lsf(b, lsb);
          indices.prepend(lsb + i * BITS_PER_CHUNK);
        }
      }
      i++;
    }
    indices.reverse();
    return indices;
  }
  public uint
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
  
  // FIXME: Should these be called foreach? It's the convention but foreach
  // is a keyword in Vala...
  public void 
  apply(IndexFunc func, pointer data)
  {
    SList<uint> indices = get_indices();
    foreach(uint index in indices)
      func(index, data);
  }
  public void
  apply_decreasing(IndexFunc func, pointer data)
  {
    SList<uint> indices = get_indices();
    indices.reverse();
    foreach(uint index in indices)
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
  assign_bit(uint i, bool val)
  {
    if (val)
      set_bit(i);
    else unset_bit(i);
  }
  // FIXME: Need to check if 'i' is in range.
  public void
  set_bit(uint i)
  {
    bits[i / BITS_PER_CHUNK] |= (uchar)(1 << (i % BITS_PER_CHUNK));
  }
  public void
  unset_bit(uint i)
  {
    bits[i / BITS_PER_CHUNK] &= (uchar)(~(1 << (i % BITS_PER_CHUNK)));
  }
  public bool
  test_bit(uint i)
  {
    return (bits[i / BITS_PER_CHUNK] & (1 << (i % BITS_PER_CHUNK))) > 0;
  }

  /* set operations */
  // TODO: not, and, or, xor, subtract, subset
}
