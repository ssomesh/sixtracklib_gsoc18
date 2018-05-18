
#ifndef _TRACH_H
#define TRACH_H

// Reference Particle implementation
#ifndef _PARTICLES_H
#define PARTICLES(p) Particle_t p
typedef struct {
    double x;
    double px;
} Particle_t;
#define X(p) p->px
#define XP(p) p->px/(1+DELTA(p))
#define YP(p) p->py/(1+DELTA(p))

#endif

// Math infrastructure
#ifndef SQ
#endif

#ifndef _MATH_H
#include <math.h>
#endif

// Tracking element memory management
#ifndef REAL_PARAM double
#endif

#ifndef REAL_TEMP double
#endif

TRACK_RETURN NS(Drift_track)(PARTICLES, REAL_PARAM length){
    REAL_TEMP xp, yp, rpp;
    rpp = RPP;
    xp  = PX * rpp;
    yp  = PY * RPP;
    X  += xp * length;
    Y  += yp * length;
    Z  += length * (RVV - (1 + (SQ(xp) + SQ(yp)) / 2));
    return 0;
};

typedef PtrDrift_t struct {
    REAL_PARAM length;
    REAL_POINTER length;
} * Drift_ptr;

typedef ELEM_MEMATTR struct {

} * Drift_ptr;



TRACK_RETURN NS(Drift_track)(PARTICLES(p), PtrDrift_t el){
    REAL_TEMP xp, yp, rpp;
    rpp = RPP(p);
    xp  = PX(p) * rpp(p);
    yp  = PY(p) * RPP(p);
    X  += xp * el->length;
    Y  += yp * el->length;
    Z  += e->length * (RVV - (1 + (SQ(xp) + SQ(yp)) / 2));
    return 0;
};

#define PARTICLES int ii, Particle_t*

#define PARTICLES(p) Particle_t *p,  int ii,


#endif
