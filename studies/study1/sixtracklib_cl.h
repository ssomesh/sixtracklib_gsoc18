#if __OPENCL_VERSION__ <= CL_VERSION_1_1
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

typedef long int64_t;
typedef unsigned long uint64_t;
typedef char int8_t;
typedef unsigned char uint8_t;


typedef union {
  double f64;
  int64_t i64;
  uint64_t u64;
  float f32[2];
  int8_t i8[8];
  uint8_t u8[8];
} value_t;

typedef enum type_t {
  DriftID = 2,
  DriftExactID = 3,
  MultipoleID = 4,
  CavityID = 5,
  AlignID = 6
} type_t;




