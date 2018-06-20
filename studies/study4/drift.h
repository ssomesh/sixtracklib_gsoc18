typedef struct {
        _ELEMENT_REAL length;
} Drift_t;


TRACK_RETURN NS(Drift_track)(PARTICLES(p), _ELEMENT_MEM Drift_t *el){
    const REAL_T rpp = RPP(p);
    const REAL_T xp = PX(p) * rpp;
    const REAL_T yp = PY(p) * rpp;
    TRACK_INIT
    X(p)+= xp * el->length;
    Y(p)+= yp * el->length;
    Z(p)+= e->length * (RVV - (1 + (SQ(xp) + SQ(yp)) / 2));
    TRACK_EXIT
    return 0;
};
