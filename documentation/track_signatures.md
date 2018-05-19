## Tracking function signatures

Signatures under considerations:

1. explicit arguments by value
```c
// in track.h
int  track_multipole(__global Particle *p, double length, ..., __global double* bal){
    ///
    p->px+=bal[0];
    ///
}

// in block.c
   track_multipole(__global Particle *p,
                mutlipole_length(elements,elemid),
                ...,
                multipole_bal(elements,elemid))
///


```
   - (-) order of arguments matters
   - (+) compact function body
   - (-) no nested structures
2. pointer to slot and accessors functions
```c
// in track.h
int track_multipole(Particle *p, __global value_t *elements, size_t elemid ){
    double length=mutlipole_length(data,elemid);
    double *bal = mutlipole_bal(data,elemid);
    ///
    p->px+=bal[0];
    ///
}

// in block.c
   track_multipole(Particle *p,  elements,  elemid )
///
...
```
   - (-) need accessors to be defined to use tracking functions therefore larger API
   - (+) support nested structures
   - (+) no additional memory and no pointer resolutions
3. structures
```c
/// in track.h
typedef struct {
    double length ;
    ///
    double * bal  __attribute__(((aligned(8)));
} Multipoles;
  

int track_multipole(Particle *p, __global *Multipole el){
...
p->x+=length*p->px*p->rpp;
*double bal=el->bal;
...
};
/// in block.c
track_multipole(p, Multipole_build(elements,elemid);
/// or ///
track_multipole(p, (_global Multipole *) &elements[elemid] );
       


```
   - (+) idiomatic
   - (-) need a way to resolve pointers, data cannot be in constant memory
   - (-) alignment has to be enforced e.g. 32bit pointers should be in 64bit slots


