/* 
= Imputation =

Imputation is used to replace missing values with plausible numbers.  This is particularly important for displays such as the tour, as there is no other reasonable way to deal with missing values apart from using complete cases. 

Each variable has a imputation type, and various parameters to that type.
Almost identical to transformation, but random (condition on symbol and colour)
so needs more information.  Could the stage be passed as a parameter to the 
transformation function?

There are currently:

	* fixed (user specified, mean, median, % below/above lowest/highest value)
	* random

And can be applied either the whole data set, or per symbol/colour group

== To do ==

 * convert each imputation type into its own object
 * probably don't need anything in this file - imputation will just be
		another transformation stage.
	
*/

public class GGobi.Stage.Impute : Stage {

}