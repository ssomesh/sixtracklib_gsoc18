import pyopencl as cl
import os

import numpy as np

class CLManager(object):
    _flags={'rw':cl.mem_flags.READ_WRITE|cl.mem_flags.COPY_HOST_PTR,
            'ro':cl.mem_flags.READ_ONLY |cl.mem_flags.COPY_HOST_PTR}
    def __init__(self,platform=0,device=0,verbose=True):
        self.platform=cl.get_platforms()[platform]
        self.device=self.platform.get_devices()[device]
        self.context=cl.Context([self.device])
        if verbose:
            os.environ['PYOPENCL_COMPILER_OUTPUT'] = '1'
        self.srcpath = '-I%s' % os.path.dirname(os.path.abspath(__file__))
        self.queue = cl.CommandQueue(self.context)
        self.buffers={}
    def compile(self,filename):
        modulepath = os.path.dirname(os.path.abspath(__file__))
        srcpath = '-I%s' % modulepath
        source=open(os.path.join(modulepath,filename)).read()
        prg=cl.Program(self.context, source).build(options=[srcpath])
        name=os.path.split(filename)[1].split('.')[0]
        setattr(self,name,prg)
        for name in prg.kernel_names.split(';'):
            k=Kernel(getattr(prg,name),self.queue)
            setattr(self,name,k)
        return self
    def buffer(self,hostbuf,flag='rw'):
        flag=self._flags.get(flag,flag)
        return cl.Buffer(self.context, flag, hostbuf=hostbuf)
    u64=np.uint64
    i64=np.int64
    f64=np.double
    def dev_to_host(self,buf):
        cl.enqueue_copy(self.queue,buf.hostbuf,buf)
    def host_to_dev(self,buf):
        cl.enqueue_copy(self.queue,buf,buf.hostbuf)

class Kernel(object):
    def __init__(self,kernel,queue):
        self.kernel=kernel
        self.queue=queue
    def __call__(self,size,*args,lsize=None):
        self.kernel(self.queue,size,lsize,*args)

ctx=CLManager(0,0).compile("code.c")

import time
def test(N,R,name='add_vec'):
   N=ctx.u64(N)
   R=ctx.u64(R)
   a=ctx.buffer(np.random.rand(N),'ro')
   b=ctx.buffer(np.random.rand(N),'ro')
   c=ctx.buffer(np.zeros(N),'rw')
   start=time.time()
   ctx.host_to_dev(c)
   kern=getattr(ctx,name)
   kern([N],N,R,a,b,c)
   ctx.dev_to_host(c)
   err=(a.hostbuf+b.hostbuf-c.hostbuf/R).std()
   return time.time()-start,err

import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as pl

def mkplot(test,name):
     out=[]
     nn=[1,2**11,2**14,2**16,2**19,2**20]
     rr=10**np.arange(0,6)
     for n in nn:
         for r in rr:
             res=test(n,r,name)[0]
             print(f"{n:>10} {r:>10} {res:8.5f}")
             out.append((n,r,res))

     nn,rr,tt=np.array(out).reshape((len(nn),len(rr),3)).T

     pl.clf()
     for ni in range(tt.shape[1]):
         pl.loglog(rr[:,ni],tt[:,ni],label=f"size: {int(nn[0,ni]):d}")

     pl.xlabel('inner loops')
     pl.ylabel('time')
     pl.legend()
     pl.title(f'{ctx.device.name} - {name}')
     pl.ylim(1e-4,10)
     pl.savefig(f'{name}.png')


mkplot(test,'add_vec')
mkplot(test,'mul_vec')
