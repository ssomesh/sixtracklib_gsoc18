from flatobject import Object, f64, u64, i64



clight = 299792458
twopi = 2*pi
radtodeg = 180/pi
degtorad = pi/180

factorial= np.r_[1,np.cumprod(range(1,21))]


class ElemId(object):
    Drift=2
    DriftExact=3
    Multipole=4
    Cavity=5
    Align=6
    BeamBeam4D=9
    BeamBeam6D=10
    PhaseTrombone=11


class Drift(Object):
    objid = u64(value=obj_ids.Drift)
    length = f64()

class DriftExact(Object):
    objid = u64(value=obj_ids.DriftExact)
    length = f64()

class Multipole(Object):
    objid = u64(value=obj_ids.Multipole)
    order   = u64(const=True)
    length  = f64()
    hxl     = f64()
    hyl     = f64()
    bal     = f64(size='2*(order+1)')
    @property
    def knl(self):
        return self.bal[0::2]/factorial[self.order]
    @property
    def ksl(self):
        return self.bal[1::2]/factorial[self.order]
    @property.setter
    def knl(self,knl):
        lknl=len(knl)
        if lknl>self.order:
            raise ValueError("knl size bigger than order")
        self.bal[0:2*lknl:2]=np.array(knl)/factorial
    @property.setter
    def ksl(self,ksl):
        lksl=len(ksl)
        if lksl>self.order:
            raise ValueError("ksl size bigger than order")
        self.bal[0:2*lksl:2]=np.array(ksl)/factorial
    def __setter__(self, knl=[], ksl=[], **nvargs):
        nvargs['order'] = max(len(knl),len(ksl))
        CObject.__init__(self, **nvargs)

class Cavity(Object):
    objid = u64(value=obj_ids.Cavity)
    voltage = f64()
    kfreq  = f64()
    lag_rad = f64()
    @property
    def frequency(self):
        return self.kfreq*c/(twopi)
    @property.setter(self):
    def frequency(self,frequency):
        return self.kfreq=twopi/c*frequency
    @property
    def lag(self):
        return self.lag_rad*radtodeg
    @property.setter(self):
    def lag(self,lag):
        return self.lag_rad=lag*degtorad

class Align(Object):
    objid = u64(value=obj_ids.Align)
    cz = f64()
    sz = f64()
    dx = f64()
    dy = f64()
    @property
    def tilt(self):
        return atan2(self.cz/self.cx)*radtodeg
    @property.setter
    def tilt(self,tilt):
        tilt_deg=tilt*degtorad
        self.cz=cos(tilt)
        self.sz=sin(tilt)


class BeamBeam4D(Object):
    objid = u64(value=obj_ids.BeamBeam4D)
    q_part = f64()
    N_part = f64()
    sigma_x = Field(BeamBeam6D,size='nslice')
    sigma_y = f64()
    beta_s  = f64()
    min_sigma_diff = f64()
    Delta_x = f64()
    Delta_y = f64()


class BeamBeam6D(Object):
    q_part =
    BB6D_boost_data =
    BB6D_Sigmas =
    min_sigma_diff =
    threshold_singular =
    N_slices =
    N_part_per_slice =  f64(size = N_slices)
    Delta_x_per_slice = f64(size = N_slices)
    Delta_y_per_slice = f64(size = N_slices)
    sigma_per_slice   = f64(size = N_slices)
    closed_orbit      = f64(size = 6)







