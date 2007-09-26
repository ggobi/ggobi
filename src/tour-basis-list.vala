/* 
= Basis List =

Loop through a list of bases.  Responsibility of user to ensure that they are 
of the correct size.

This basis generation method ignores frozen variables.

*/
using GLib;

public class GGobi.TourBasisList : TourBasis {
  private SList<TourMatrix> bases;
  private weak SList<TourMatrix> current_basis;

  override TourMatrix generate(TourState[] states) {
    current_basis = current_basis.next;
    if (current_basis == null) {
      current_basis = bases;
    }
    
    TourMatrix next = ((TourMatrix) current_basis.data).copy();
    return next;
  }
  
  // Methods delegated to underlying SList
  public void append(TourMatrix# data) {
    bases.append(data);
  }
  public void prepend(TourMatrix# data) {
    bases.prepend(data);
  }
  public void insert(TourMatrix# data, int pos) {
    bases.insert(data, pos);
  }
  public void remove(TourMatrix data) {
    bases.remove(data);
  }
  public uint length() {
    return bases.length();
  }
  
}