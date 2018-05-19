#define _ELEMENT_MEM_TYPES
#define _ELEMENT_MEM __global
#define _ELEMENT_REAL double
#define TRACK_RETURN int

#define _PARTICLES_MEM_TYPES
#define _ELEMENT_MEM
#define _PARTICLE_REAL double *

typedef unsigned long int uint64_t;

#include "particles.h"
#include "track.h"


__kernel Drift(__global PARTICLES(p),
               uint64_t nparts,
               uint64_t groupsize,
               __global 

