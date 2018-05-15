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
