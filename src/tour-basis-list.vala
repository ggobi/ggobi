/* 
= Basis List =

Loop through a list of bases.  Responsibility of user to ensure that they are 
of the correct size.

*/

public class GGobi.TourBasisList : TourBasis {
  SList<TourMatrix> bases;
  SList<TourMatrix> current_basis;

  // Methods delegated to underlying SList
  public void append(TourMatrix# data) bases.append(data);
  public void prepend(TourMatrix# data) bases.prepend(data);
  public void insert(TourMatrix# data, int pos) bases.insert(data, pos);
  public void remove(TourMatrix data) bases.remove(data);
  public uint length() bases.length();
  
  override TourMatrix generate(TourState[] states) {
    current_basis = current_basis.next;
    if (current == null) {
      current_basis = bases;
    }
    
    return current_basis.data;
  }
}