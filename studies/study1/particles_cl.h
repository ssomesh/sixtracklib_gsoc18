#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define M_2PI 6.283185307179586
#define SQ(f) (f)*(f)
#define CLIGHT 299792458


#ifndef REAL_P
#define REAL_PARAM double
#define REAL_PARAM_P __global double*
#define REAL_COORD double
#define REAL_TEMP double
#define INT_PARAM int
#endif

#ifndef TRACK_RETURN
#define TRACK_RETURN int
#endif

#ifndef NS
#define NS(f) f
#endif


#ifndef PARTICLES
#define PARTICLES double* p
#define X       p[ 0]
#define PX      p[ 1]
#define Y       p[ 2]
#define PY      p[ 3]
#define Z       p[ 4]
#define DELTA   p[ 5]
#define CHI     p[ 6]
#define P0C     p[ 7]  //eV
#define MASS0   p[ 8]  //eV
#define BETA0   p[ 9]
#define STABLE  p[10]
#define PC      P0C*(1+DELTA)  // P0/P
#define ENERGY  sqrt(SQ(PC) + SQ(MASS0))
#define BETA    PC/ENERGY
#define RPP     1/(1+DELTA)    // P0/P
#define RVV     PC/ENERGY/BETA0
#endif




