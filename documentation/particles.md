##Particles 



Each particles is described by a set of coordinates. 

### Coordinate definitions

There are 6 independent coordinates manipulated by the tracking functions.

Tracking functions needs also additional parameters.

Auxiliary coordinates can be addeded to avoid recomputations



### Coordinate memory structures

Coordinates may be allocated in

* Array of structures: each particle coordinate is close to each other
* Structure of array: computation can be vectorized more easily
* Array of Structures of array: in case there are many particles, coordinates might be too distant resulting in cache misses

