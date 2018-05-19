#if defined(cl_khr_fp64)
#if __OPENCL_VERSION__ <= CL_VERSION_1_1
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif
#elif defined(cl_amd_fp64)
#  pragma OPENCL EXTENSION cl_amd_fp64: enable
#else
#  error double precision is not supported
#endif


__kernel void add_vec(
       ulong n,
       ulong r,
       __global const double *a,
       __global const double *b,
       __global double *c
       )
{
    size_t i = get_global_id(0);
    double cc=0;
    if (i < n) {
       for (int jj=0; jj<r; jj++){
           cc+= a[i] + b[i];
       };
       c[i]=cc;
    }
};

__kernel void mul_vec(
       ulong n,
       ulong r,
       __global const double *a,
       __global const double *b,
       __global double *c
       )
{
    size_t i = get_global_id(0);
    double cc=0;
    if (i < n) {
       for (int jj=0; jj<r; jj++){
           cc*= a[i] * b[i];
       };
       c[i]=cc;
    }
};

