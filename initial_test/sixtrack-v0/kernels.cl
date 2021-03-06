#if defined(cl_khr_fp64)
#  pragma OPENCL EXTENSION cl_khr_fp64: enable
#elif defined(cl_amd_fp64)
#  pragma OPENCL EXTENSION cl_amd_fp64: enable
#else
# error double precision is not supported
#endif
#include "sixtracklib/_impl/namespace_begin.h"
#include "sixtracklib/_impl/definitions.h"
#include "sixtracklib/common/blocks.h"
#include "sixtracklib/common/impl/particles_type.h"
#include "sixtracklib/common/impl/particles_api.h"
#include "sixtracklib/common/particles.h"
#include "sixtracklib/common/impl/beam_elements_type.h"
#include "sixtracklib/common/impl/beam_elements_api.h"
#include "sixtracklib/common/beam_elements.h"
kernel void unserialize(
                        global uchar *copy_buffer, // uint8_t is uchar
                        global uchar *copy_buffer_particles, // uint8_t is uchar
                        ulong NUM_PARTICLES
                        )
{
size_t gid = get_global_id(0);
NS(Blocks) copied_beam_elements;
NS(Blocks_preset)( &copied_beam_elements ); // very important for initialization
int ret = NS(Blocks_unserialize)(&copied_beam_elements, copy_buffer);
NS(Blocks) copied_particles_buffer;
NS(Blocks_preset) (&copied_particles_buffer);

ret = NS(Blocks_unserialize)(&copied_particles_buffer, copy_buffer_particles);
}


kernel void track_drift_particle(
                                 global uchar *copy_buffer, // uint8_t is uchar
                                 global uchar *copy_buffer_particles, // uint8_t is uchar
                                 ulong NUM_PARTICLES,
                                 ulong NUM_TURNS // number of times a particle is mapped over each of the beam_elements
                                 )
{
  NS(block_num_elements_t) ii = get_global_id(0);
  if(ii >= NUM_PARTICLES) return;

  /* For the particles */
  NS(Blocks) copied_particles_buffer;
  NS(Blocks_preset) (&copied_particles_buffer);

  int ret = NS(Blocks_unserialize)(&copied_particles_buffer, copy_buffer_particles);
  SIXTRL_GLOBAL_DEC st_BlockInfo const* it  =  // is 'it' pointing to the outer particles? check.
        st_Blocks_get_const_block_infos_begin( &copied_particles_buffer );
  SIXTRL_GLOBAL_DEC NS(Particles) const* particles = 
        ( SIXTRL_GLOBAL_DEC st_Particles const* )it->begin; 

  // *particles now points to the first 'outer' particle
  // @ Assuming only a single outer particle
  // each 'ii' refers to an inner particle

  /* for the beam element */
  NS(Blocks) copied_beam_elements;
  NS(Blocks_preset)( &copied_beam_elements ); // very important for initialization
  ret = NS(Blocks_unserialize)(&copied_beam_elements, copy_buffer);

  SIXTRL_STATIC SIXTRL_REAL_T const ONE      = ( SIXTRL_REAL_T )1;
  SIXTRL_STATIC SIXTRL_REAL_T const ONE_HALF = ( SIXTRL_REAL_T )0.5L;

  // for each particle we apply the beam_elements, as applicable (decided by the switch case)

  for (size_t nt=0; nt < NUM_TURNS; ++nt) {
    SIXTRL_GLOBAL_DEC st_BlockInfo const* belem_it  = 
        st_Blocks_get_const_block_infos_begin( &copied_beam_elements );
    SIXTRL_GLOBAL_DEC st_BlockInfo const* belem_end =
        st_Blocks_get_const_block_infos_end( &copied_beam_elements );

		for( ; belem_it != belem_end ; ++belem_it )
				 {
					 st_BlockInfo const info = *belem_it;
					 NS(BlockType) const type_id =  st_BlockInfo_get_type_id(&info );
					 switch( type_id )
					 {
						 case st_BLOCK_TYPE_DRIFT:
						 {
							 __global st_Drift const* drift = 
							 st_Blocks_get_const_drift( &info );
							 st_Drift const drift_private = *drift;
							 SIXTRL_REAL_T const length = st_Drift_get_length( &drift_private );  
							 SIXTRL_REAL_T const rpp = particles->rpp[ii]; 
							 SIXTRL_REAL_T const px = particles->px[ii] * rpp; 
							 SIXTRL_REAL_T const py = particles->py[ii] * rpp; 
							 SIXTRL_REAL_T const dsigma = 
							 ONE - particles->rvv[ii]  * ( ONE + ONE_HALF * ( px * px + py * py ) );
							 SIXTRL_REAL_T sigma = particles->sigma[ii];
							 SIXTRL_REAL_T s = particles->s[ii];
							 SIXTRL_REAL_T x = particles->x[ii];
							 SIXTRL_REAL_T y = particles->y[ii];
							 sigma += length * dsigma;
							 s     += length;
							 x     += length * px;
							 y     += length * py;
							 particles->s[ ii ] = s;
							 particles->x[ ii ] = x;
							 particles->y[ ii ] = y;
							 particles->sigma[ ii ] = sigma;
							 break;
						 }
						 default:
						 {
							 printf("unknown     | --> skipping\n");
						 }
					 };
				 }
	}

     };

