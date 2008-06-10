/*
== Screen scaling ==
(`plane_to_screen`)

Once data has been flattened on to a plane, it is necessary to scale the x and
y positions to fit on a screen. There are two special considerations here:

# Points should not be plotted close to the edges of the plot.
# Scales need to be smoothly adaptive as data changes

Cleveland suggests that points should not be plotted next to the edges of a
plot, but there should be a margin between the data and the plot. (GGobi2
`limits_adjust`, 10% of current plot)

By default, scales should be adaptive: as points move outside the existing
range, they should be rescaled (but it should not automatically rescale if the
range decreases). Also need the option to freeze the current scaling, and to
reset it to the current data. (GGobi2: adhoc solutions in `tour1d_projdata`,
`tour2d_projdata`)

*/

using GLib;

public class GGobi.StageScreen : Stage {
       
     
     /* Data ranges */
     double minx;
     double miny;
     double maxx;
     double maxy;
	
     /* Dimensions of the screen */
     uint width;
     uint height;
	
     /* Is the scaling currently fixed, or free? */
     bool fixed;

     /* Reset data ranges */
     public void reset() {
       minx = miny = double.MAX;
       maxx = maxy = double.MIN;
     }
     
     public void update(Variable x, Variable y) {
       minx = Math.fmin(minx, x.get_min());
       miny = Math.fmin(miny, y.get_min());
       maxx = Math.fmax(maxx, x.get_max());
       maxy = Math.fmax(maxy, y.get_max());
     }
     
     /* Width of margin, in pixels.  
	
	This is a heuristic that Hadley thinks should work well.
	It uses the minimum of 10 pixels, or 10% of the smaller of height 
	or width */
     uint margin() {
       return (uint) Math.fmax(10, Math.fmin(height, width) / 100);
     }
     
     override double get_raw_value(uint i, uint j) {
       return parent.get_raw_value(i, j);
     }

     override void set_raw_value(uint i, uint j, double value) {
       parent.set_raw_value(i, j, value);
     }
}