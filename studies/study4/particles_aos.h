#ifndef _PARTICLES_H
#define _PARTICLES_H

typedef struct {
    PARTICLE_REAL x;
    PARTICLE_REAL px;

} Particle_t;

#define PARTICLES(p) Particle_t *p
//independent coordinates
#define X(p) p->x
#define Y(p) p->y
#define PX(p) p->px
#define PY(p) p->py

//dependent coordinates
#define XP(p) p->px/(1+DELTA(p))
#define YP(p) p->py/(1+DELTA(p))


#endif
