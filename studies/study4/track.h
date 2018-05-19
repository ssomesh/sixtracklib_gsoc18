#ifndef _TRACKH_H
#define TRACKH_H

// Reference Particle implementation
#ifndef _PARTICLES_H
#include "particles.h"
#endif

// Math infrastructure
#ifndef _MATH_H
#include <math.h>
#endif

#ifndef SQ
#define SQ(x) (x)*(x)
#endif

// Tracking data types
#ifndef _ELEMENT_MEM_TYPES
#define _ELEMENT_MEM
#define _ELEMENT_REAL double
#define TRACK_RETURN int
#endif

// Namespace
#ifndef NS
#define NS(name) name
#endif

#include "drift.h"


#endif
