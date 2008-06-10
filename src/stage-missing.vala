/* 
= Missing stage =

This stage converts all data in to a binary matrix of zero's and one's.  This
replaces the functionaliy previously available in missing.c

TODO: Return categorical variables with values present and missing.

*/
using GLib;

public class GGobi.StageMissing : Stage {
  
  override double get_raw_value(uint i, uint j) {
    Variable v = get_variable(j);
    if (v.is_attribute) return parent.get_raw_value(i, j);
    
    return (double) parent.is_missing(i, j);
  }

  override void set_raw_value(uint i, uint j, double value) {
    return;
  }  
  
}