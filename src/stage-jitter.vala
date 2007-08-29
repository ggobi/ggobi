/* 
= Jittering =

When displaying variables with few unique values (eg. discrete) on a scatterplot, it is useful to add a small amount of random jitter. 

In GGobi2, jittering occured after the world transformation, so it could assume that the range of each variable was [0, 1].  This is not the case in GGobi3, so the range of the variable is used explicity. 

For each jittered variable, store:

 * distribution (normal or uniform)
 * jitter amount (between 0 and 1)

== To do ==

To do:
 * extract code for generating a random normal 
 * datastructure to store dist, amount and cache for each variable

Jit value = original * (1 - amount) + random [-1, 1] * range * amount

Note: jittering amounts are not cached, so when modifying jitter points,
they are not "unjittered" prior to modification.
	
*/

public class GGobi.Stage.Jitter : Stage {
	
}