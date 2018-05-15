#include "sixtracklib_cl.h"
#include "particles_cl.h"
#include "track.h"

double Drift_length(__global value_t *elements,
                    __global uint64_t element_id){
     return elements[id + 1].f64
};

double Multiple_order(__global value_t *elements,
                      __global uint64_t element_id){
    return elements[id + 1].u64
};

double Multiple_length(__global value_t *elements,
                       __global uint64_t element_id){
    return elements[id + 2].f64
};

double Multiple_hxl(__global value_t *elements,
                    uint64_t element_id){
    return elements[id + 3].u64
};

double Multiple_hyl(__global value_t *elements,
                    uint64_t element_id){
    return elements[id + 4].u64
};

__global double *Multiple_bal(__global value_t *elements,
                              uint64_t element_id){
return &elements[id + 5].f64;
};

__kernel void Elements_track(__global value_t *elements,
                             __global uint64_t *element_ids,
                             uint64_t nelems,
                             uint64_t nturns,
                             __global value_t *particles) {
  uint64_t partid = get_global_id(0);
}
