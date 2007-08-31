/*
= Texturing =

Texturing is a slightly more sophisticated version of jittering, where points
that don't need to be jittered (because they don't overlap with other points)
aren't.

This version currently recomputes random numbers at every call, and is not
appropriate for dynamic displays.

*/

public class GGobi.Stage.Texture : Stage {
	
}