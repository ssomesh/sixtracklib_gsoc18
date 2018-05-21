from cffi import FFI
import numpy as np

ffi = FFI()

#define structure
ffi.cdef("""
typedef struct t_point t_point;
struct t_point
{
  double x;
  double y;
};
""")

# allocate structure
p = ffi.new("t_point *")

assert ffi.offsetof('t_point','y')==8

y=ffi.addressof(p,'y')
y[0]=3
p.y==3

# use allocate memory to write to structure
data=np.zeros(10)
p2=ffi.cast('t_point *',data.ctypes.data)

p2.x=4
p2[3].x=5
assert data[0]==4.0
assert data[6]==5.0

