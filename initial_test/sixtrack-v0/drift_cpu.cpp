#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <random>
#include <vector>

#include "sixtracklib/_impl/definitions.h"
#include "sixtracklib/_impl/path.h" // for NS(PATH_TO_BASE_DIR)
#include "sixtracklib/common/blocks.h"
#include "sixtracklib/common/beam_elements.h"
#include "sixtracklib/common/particles.h"


  void track_drift_particles(NS(Blocks)& beam_elements, NS(Blocks)& particles_buffer, NS(block_size_t) NUM_PARTICLES, NS(block_size_t) NUM_TURNS) {

  st_BlockInfo const* it  =  // is 'it' pointing to the outer particles? check.
        st_Blocks_get_const_block_infos_begin( &particles_buffer );
  NS(Particles) const* particles = 
        ( st_Particles const* )it->begin; 

  // *particles now points to the first 'outer' particle
  // @ Assuming only a single outer particle
  // each 'ii' refers to an inner particle

  /* for the beam element */


  SIXTRL_STATIC SIXTRL_REAL_T const ONE      = ( SIXTRL_REAL_T )1;
  SIXTRL_STATIC SIXTRL_REAL_T const ONE_HALF = ( SIXTRL_REAL_T )0.5L;

  // for each particle we apply the beam_elements, as applicable (decided by the switch case)

for (size_t ii=0; ii < NUM_PARTICLES; ++ii) {

  for (size_t nt=0; nt < NUM_TURNS; ++nt) {

  st_BlockInfo const* belem_it  = 
      st_Blocks_get_const_block_infos_begin( &beam_elements );
  st_BlockInfo const* belem_end =
      st_Blocks_get_const_block_infos_end( &beam_elements );

		for( ; belem_it != belem_end ; ++belem_it )
				 {
					 st_BlockInfo const info = *belem_it;
					 NS(BlockType) const type_id =  st_BlockInfo_get_type_id(&info );
					 switch( type_id )
					 {
						 case st_BLOCK_TYPE_DRIFT:
						 {
							  st_Drift const* drift = 
							 st_Blocks_get_const_drift( &info );
							 SIXTRL_REAL_T const length = st_Drift_get_length( drift );  
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
  }
  //return particles_buffer;
}

int main()
{
    /* We will use 9+ beam element blocks in this example and do not 
     * care to be memory efficient yet; thus we make the blocks for 
     * beam elements and particles big enough to avoid running into problems */
    
    constexpr st_block_size_t const MAX_NUM_BEAM_ELEMENTS       = 20u;
    constexpr st_block_size_t const NUM_OF_BEAM_ELEMENTS        = 9u;
    
    /* 1MByte is plenty of space */
    constexpr st_block_size_t const BEAM_ELEMENTS_DATA_CAPACITY = 1048576u; 
    
    /* Prepare and init the beam elements buffer */
    
    st_Blocks beam_elements;    
    st_Blocks_preset( &beam_elements );
    
    int ret = st_Blocks_init( &beam_elements, MAX_NUM_BEAM_ELEMENTS, 
                              BEAM_ELEMENTS_DATA_CAPACITY );
    
    assert( ret == 0 ); /* if there was an error, ret would be != 0 */
    
    /* Add NUM_OF_BEAM_ELEMENTS drifts to the buffer. For this example, let's
     * just have one simple constant length for all of them: */
    
    for( st_block_size_t ii = 0 ; ii < NUM_OF_BEAM_ELEMENTS ; ++ii )
    {
        double const drift_length = double{ 0.2L };
        st_Drift* drift = st_Blocks_add_drift( &beam_elements, drift_length );
        
        assert( drift != nullptr ); /* Otherwise, there was a problem! */
    }
    
    /* Check if we *really* have the correct number of beam elements and 
     * if they really are all drifts */
    
    assert( st_Blocks_get_num_of_blocks( &beam_elements ) == 
            NUM_OF_BEAM_ELEMENTS );
    
    /* The beam_elements container is currently not serialized yet -> 
     * we could still add blocks to the buffer. Let's just do this and 
     * add a different kind of beam element to keep it easier apart! */
    
//    st_DriftExact* drift_exact = st_Blocks_add_drift_exact( 
//        &beam_elements, double{ 0.1 } );
//    
//    assert( drift_exact != nullptr );
    
    assert( st_Blocks_get_num_of_blocks( &beam_elements ) == 
            ( NUM_OF_BEAM_ELEMENTS) );
    
    /* Always safely terminate pointer variables pointing to resources they
     * do not own which we no longer need -> just a good practice */
    
//    drift_exact = nullptr;
    
    /* After serialization, the "structure" of the beam_elements buffer is 
     * frozen, but the data in the elements - i.e. the length of the 
     * individual drifts in our example - can still be modified. We will 
     * just not be able to add further blocks to the container */
    
    assert( !st_Blocks_are_serialized( &beam_elements ) );
    
    ret = st_Blocks_serialize( &beam_elements );
    
    assert( ret == 0 );
    assert( st_Blocks_are_serialized( &beam_elements ) ); // serialization on CPU done.
    
    /* Next, let's iterate over all the beam_elements in the buffer and 
     * print out the properties -> we expect that NUM_OF_BEAM_ELEMENTS
     * st_Drift with the same length appear and one st_DriftExact with a 
     * different length should appear in the end */
    
    std::cout << "\r\n"
              << "Print these newly created beam_elements: \r\n"
              << "\r\n";
    
    st_block_size_t ii = 0;
    
    /* Generate an iterator range over all the stored Blocks: */
    
    st_BlockInfo const* belem_it  = 
        st_Blocks_get_const_block_infos_begin( &beam_elements );
        
    st_BlockInfo const* belem_end =
        st_Blocks_get_const_block_infos_end( &beam_elements );
        
    for( ; belem_it != belem_end ; ++belem_it, ++ii )
    {
        std::cout << std::setw( 6 ) << ii << " | type: ";
        
        auto const type_id = st_BlockInfo_get_type_id( belem_it );
        
        switch( type_id )
        {
            case st_BLOCK_TYPE_DRIFT:
            {
                st_Drift const* drift = 
                    st_Blocks_get_const_drift( belem_it );
                
                std::cout << "drift        | length = "
                          << std::setw( 10 ) 
                          << st_Drift_get_length( drift )
                          << " [m] \r\n";
                            
                break;
            }
            
            case st_BLOCK_TYPE_DRIFT_EXACT:
            {
                st_DriftExact const* drift_exact =
                    st_Blocks_get_const_drift_exact( belem_it );
                
                std::cout << "drift_exact  | length = "
                          << std::setw( 10 )
                          << st_DriftExact_get_length( drift_exact )
                          << " [m] \r\n";
                          
                break;
            }
            
            default:
            {
                std::cout << "unknown     | --> skipping\r\n";
            }
        };
    }
    
    std::cout.flush();

   //////////////////// Particles //////////////////////////

    st_block_size_t const NUM_PARTICLE_BLOCKS     = 1u;
    st_block_size_t const PARTICLES_DATA_CAPACITY = 1048576u;
    st_block_size_t const NUM_PARTICLES           = 100u;
    
    st_Blocks particles_buffer;
    st_Blocks_preset( &particles_buffer );
    
    ret = st_Blocks_init( 
        &particles_buffer, NUM_PARTICLE_BLOCKS, PARTICLES_DATA_CAPACITY );
    
    assert( ret == 0 );
    
    st_Particles* particles = st_Blocks_add_particles( 
        &particles_buffer, NUM_PARTICLES );
    
    if( particles != nullptr )
    {
        /* Just some random values assigned to the individual attributes
         * of the acutal particles -> these values do not make any 
         * sense physically, but should be safe for calculating maps ->
         * please check with the map for drift whether they do not produce
         * some NaN's at the sqrt or divisions by 0 though!*/
        
        std::mt19937_64  prng( 20180622 );
        
        std::uniform_real_distribution<> x_distribution(  0.05, 1.0 );
        std::uniform_real_distribution<> y_distribution(  0.05, 1.0 );
        std::uniform_real_distribution<> px_distribution( 0.05, 0.2 );
        std::uniform_real_distribution<> py_distribution( 0.05, 0.2 );
        std::uniform_real_distribution<> sigma_distribution( 0.01, 0.5 );
        
        assert( particles->s     != nullptr );
        assert( particles->x     != nullptr );
        assert( particles->y     != nullptr );
        assert( particles->px    != nullptr );
        assert( particles->py    != nullptr );
        assert( particles->sigma != nullptr );
        assert( particles->rpp   != nullptr );
        assert( particles->rvv   != nullptr );
        
        assert( particles->num_of_particles == NUM_PARTICLES );
        
        for( st_block_size_t ii = 0 ; ii < NUM_PARTICLES ; ++ii )
        {
            particles->s[ ii ]     = 0.0;
            particles->x[ ii ]     = x_distribution( prng );
            particles->y[ ii ]     = y_distribution( prng );
            particles->px[ ii ]    = px_distribution( prng );
            particles->py[ ii ]    = py_distribution( prng );
            particles->sigma[ ii ] = sigma_distribution( prng );
            particles->rpp[ ii ]   = 1.0;
            particles->rvv[ ii ]   = 1.0;
        }
    }
    
    ret = st_Blocks_serialize( &particles_buffer );
    assert( ret == 0 );

    st_BlockInfo const* it  = 
        st_Blocks_get_const_block_infos_begin( &particles_buffer );
    
    st_BlockInfo const* end =
        st_Blocks_get_const_block_infos_end( &particles_buffer );
    
    for( ; it != end ; ++it )
    {
        st_Particles const* particles = 
            ( st_Particles const* )it->begin;
            
        std::cout.precision( 4 );
        
        for( st_block_size_t ii = 0 ; ii < NUM_PARTICLES ; ++ii )
        {
            std::cout << " ii    = " << std::setw( 6 ) << ii
                      << std::fixed
                      << " | s     = " << std::setw( 6 ) << particles->s[ ii ]
                      << " | x     = " << std::setw( 6 ) << particles->x[ ii ]
                      << " | y     = " << std::setw( 6 ) << particles->y[ ii ]
                      << " | px    = " << std::setw( 6 ) << particles->px[ ii ]
                      << " | py    = " << std::setw( 6 ) << particles->py[ ii ]
                      << " | sigma = " << std::setw( 6 ) << particles->sigma[ ii ]
                      << " | rpp   = " << std::setw( 6 ) << particles->rpp[ ii ]
                      << " | rvv   = " << std::setw( 6 ) << particles->rvv[ ii ]
                      << "\r\n";
        }
    }
    
    std::cout.flush();

    st_block_size_t NUM_TURNS = 10;
   // NS(Blocks) particles_buffer_copy;
//    NS(Blocks_preset) (&particles_buffer_copy);
 // NS(Blocks) particles_buffer_copy =  
  track_drift_particles(beam_elements, particles_buffer, NUM_PARTICLES, NUM_TURNS);


// After applying the track_drift_particles map to  Particles
std::cout << "After applying the track_drift_particles map to  Particles:" << std::endl;

    st_BlockInfo const* itr  = 
        st_Blocks_get_const_block_infos_begin( &particles_buffer );
    
     st_BlockInfo const* endr =
        st_Blocks_get_const_block_infos_end( &particles_buffer );
    
    for( ; itr != endr ; ++itr )
    {
        st_Particles const* particles = 
            ( st_Particles const* )itr->begin;
            
        std::cout.precision( 4 );
        
        for( st_block_size_t ii = 0 ; ii < NUM_PARTICLES ; ++ii )
        {
            std::cout << " ii    = " << std::setw( 6 ) << ii
                      << std::fixed
                      << " | s     = " << std::setw( 6 ) << particles->s[ ii ]
                      << " | x     = " << std::setw( 6 ) << particles->x[ ii ]
                      << " | y     = " << std::setw( 6 ) << particles->y[ ii ]
                      << " | px    = " << std::setw( 6 ) << particles->px[ ii ]
                      << " | py    = " << std::setw( 6 ) << particles->py[ ii ]
                      << " | sigma = " << std::setw( 6 ) << particles->sigma[ ii ]
                      << " | rpp   = " << std::setw( 6 ) << particles->rpp[ ii ]
                      << " | rvv   = " << std::setw( 6 ) << particles->rvv[ ii ]
                      << "\r\n";
        }
    }
    
    std::cout.flush();




    st_Blocks_free( &particles_buffer );
//    st_Blocks_free( &particles_buffer_copy );
    return 0;
}
