#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>

#include "sixtracklib/_impl/definitions.h"
#include "sixtracklib/common/blocks.h"
#include "sixtracklib/common/particles.h"

int main()
{
    st_block_size_t const NUM_PARTICLE_BLOCKS     = 1u;
    st_block_size_t const PARTICLES_DATA_CAPACITY = 1048576u;
    st_block_size_t const NUM_PARTICLES           = 100u;
    
    st_Blocks particles_buffer;
    st_Blocks_preset( &particles_buffer );
    
    int ret = st_Blocks_init( 
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
    
    /* ===================================================================== */
    /* Copy to other buffer to simulate working on the GPU */
    
    std::vector< uint8_t > copy_buffer( 
        st_Blocks_get_const_data_begin( &particles_buffer ), 
        st_Blocks_get_const_data_end( &particles_buffer ) );
    
    st_Blocks copy_particles_buffer;
    st_Blocks_preset( &copy_particles_buffer );
    
    ret = st_Blocks_unserialize( &copy_particles_buffer, copy_buffer.data() );
    assert( ret == 0 );
    
    /* on the GPU, these pointers will have __global as a decorator */
    
    SIXTRL_GLOBAL_DEC st_BlockInfo const* it  = 
        st_Blocks_get_const_block_infos_begin( &copy_particles_buffer );
    
    SIXTRL_GLOBAL_DEC st_BlockInfo const* end =
        st_Blocks_get_const_block_infos_end( &copy_particles_buffer );
    
    for( ; it != end ; ++it )
    {
        SIXTRL_GLOBAL_DEC st_Particles const* particles = 
            ( SIXTRL_GLOBAL_DEC st_Particles const* )it->begin;
            
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
    st_Blocks_free( &copy_particles_buffer );
    
    return 0;
}

/* end:  */
