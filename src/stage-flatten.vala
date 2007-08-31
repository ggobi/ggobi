/*
== Flattening (selection and projection) ==
(`world_to_plane`)

To render on a display, data must be flattened into 2d. There are two principle ways of
doing this in GGobi: selection and projection. Selection reduces the number of columns in
the data to 1 or 2, while projection multiplies the data matrix by an orthonormal
projection matrix with 1 or 2 columns. (Selection is a special case of projection but is
implemented separately for performance reasons)

There are currently four flattenning stages:
	* select-1d (`p1d_reproject`), select-2d (`xy_reproject` )
	* project-1d (`tour1d_projdata`), project-2d (`tourcorr_projdata` `tour2d_projdata` `tour2d3_projdata`)
	
*/

public class GGobi.Stage.Flatten : Stage {
	
}

public class GGobi.Stage.Project : Stage.Flatten {}
public class GGobi.Stage.Project1d : Stage.Project {}
public class GGobi.Stage.Project2d : Stage.Project {}

public class GGobi.Stage.Select : Stage.Flatten {}
public class GGobi.Stage.Select1d : Stage.Select {}
public class GGobi.Stage.Select2d : Stage.Select {}
