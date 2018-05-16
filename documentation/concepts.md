# SixTrackLib Concepts

[TOC]

## Main loop

Particles are tracked in main loop. There two type of loops, one supposedly faster for "static" simulations and another for more "dynamic"  simulations

### Static loop case

The main loop maybe described with the following code

```python
def track(particles, elements, element_list, nturns, turnbyturn, elementbyelement):
    for tt in range(nturns):
        if turnbyturn:
            turnbyturn.add(particles)
        for element_id in element_list:
            element=elements[element_id]
            track_function=track[typeid(element)]
            for ii in range(len(particles)):
                if not_lost(particle[ii]):
                     particle[ii]=track_function(particle[ii],element)
            if elementbyelement:
                elementbyelement.add(particles)

    return particles, turnbyturn, elementbyelement
```

- `particle[ii]` contains an array of coordinates for the particle `ii` that get updated in place as long as the particle is not lost;
- `track[typeid(element)]` contains a tracking function of a given type;
- `element` contains the type and the parameters for the tracking function of the element `jj`;
- `elements` contains a set of elements that could be used;
- the loop runs for a predefined sequence of elements;
- `track_function` has no side effect
- The entire loop can be coded in a single GPU kernel. The advantage is no overhead for kernel launch. The disadvantage is the register pressure which is dominated by  the most complicated `track_function`.
- `elements` may be stored in constant memory.
- Each particle maybe stored in the private memory during tracking and copied from/to globabl memory at the beginning/end of the tracking.

### Dynamic loop case:

```python
def track(particles, elements, element_list, nturns, turnbyturn, elementbyelement):
    for tt in range(nturns):
        for element_id in elements:
            element=elements[elemid]
            track_function=track[typeid(elem)]
            track_function(particles, elements, elemid)
    return particles, elements, turnbyturn, elementbyelement
```

- elements might be modified by the particles
- new particles might be generated
- this time `track_function` may have side effects. Each track_function needs to be an individual kernel or a barrier is needed after each call.

## Tracking function signatures

Signatures under consideration:

1. explicit arguments by value
```c
track_multipole(Particles, double length, ..., __global double* bal)
```
   - (-) order of arguments matters
   - (+) compact function body
   - (-) no nested structures
2. pointer to slot and accessors functions
```c
track_multiple(Particles, __global value_t *elements, size_t elemid )
length=mutlipole_length(data,elemid);
```
   - (-) need accessor to be defined, larger API
   - (+) support nested structures
   - (-) no additional memory
3. structures
```c
track_multipole(Particles, __global *Multipole el){
...
double length=el->length;
*double bal=length->el->bal;
...
```
   - (+) idiomatic
   -  (-) need storage for allocating structures unless empty slots are allocated for pointers and compilers factor out structures

