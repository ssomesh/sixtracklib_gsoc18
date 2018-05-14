from __future__ import absolute_import, print_function
import numpy as np
import pyopencl as cl

class Particles(object):
    def __init__(self,nparticles=1,ndim=10):
        self.nparticles=nparticles
        self.ndim=ndim
        self.data=np.zeros(nparticles*ndim,dtype=np.float64)

class Elements(object):
    value_t = np.dtype({'names'  :['f64','i64','u64'],
                        'formats':['<f8','<i8','<u8'],
                        'offsets':[    0,    0,    0],
                        'itemsize':8})
    DriftId=1
    MultipoleId=2
    def __init__(self,size=40000):
        self.size=size
        self.data=np.zeros(size,dtype=self.value_t)
        self.last=0
        self.elements=[]
    def add_drift(self,length=0.0):
        self.elements.append(self.last)
        self.data['u64'][self.last  ]=self.DriftId
        self.data['f64'][self.last+1]=length
        self.last+=2
    def add_multipole(self,knl=[],ksl=[],length=0.0,hxl=0.0,hyl=0.0):
        self.elements.append(self.last)
        order=max(len(knl),len(ksl))
        self.data['u64'][self.last+0]=self.MultipoleId
        self.data['u64'][self.last+1]=order
        self.data['u64'][self.last+2]=length
        self.data['u64'][self.last+3]=hxl
        self.data['u64'][self.last+4]=hyl
        fact=1
        for nn in range(len(knl)):
           self.data['f64'][self.last+5+nn*2]=knl[nn]/fact
           fact*=nn+1
        fact=1
        for nn in range(len(ksl)):
           self.data['f64'][self.last+5+nn*2+1]=ksl[nn]/fact
           fact*=nn+1
        self.last+=5*2*order

particles=Particles(nparticles=2560,ndim=10)
elements=Elements()


class CLTrack(object):
    def __init__(self,device="0.0",particles,elements):
        self.ctx = cl.create_some_context(answers=device)
        self.queue = cl.CommandQueue(ctx)
        ro=cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR
        rw=cl.mem_flags.READWRITE | cl.mem_flags.COPY_HOST_PTR
        self.particles_g = cl.Buffer(ctx, rw, hostbuf=a_np)
        self.elements_g = cl.Buffer(ctx, ro, hostbuf=a_np)






