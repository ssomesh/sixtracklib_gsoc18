/*
* TODO:
* 1. serialize on the CPU -- done
  2. copy data to  the GPU
  3. unserailize (remap) on the GPU -- use a kernel launched with 1 thread to do this
  4. launch a kernel to perform the computation in parallel on the GPU (this is the actual computation)
  5. copy the result back to the cpu -- done !
  6. unserialize the result on the CPUs -- done !

  Done!
 // IMP: The idea is to pass the memory (i.e. the serialzed array of type unsigned char) to the kernel and to reconstruct the NS(Blocks) instance then from there.
  So, NS(Blocks) needs to be reconstructed on the GPU and not passed to the GPU

*/


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

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#if 0

typedef struct NS(ParticlesSpecial)
{

    SIXTRL_GLOBAL_DEC SIXTRL_REAL_T* SIXTRL_RESTRICT 
        s      __attribute__(( aligned( 8 ) ));     /* [m] */
        
    SIXTRL_GLOBAL_DEC SIXTRL_REAL_T* SIXTRL_RESTRICT 
        x      __attribute__(( aligned( 8 ) ));     /* [m] */
    
    SIXTRL_GLOBAL_DEC SIXTRL_REAL_T* SIXTRL_RESTRICT 
        y      __attribute__(( aligned( 8 ) ));     /* [m] */
        
    SIXTRL_GLOBAL_DEC SIXTRL_REAL_T* SIXTRL_RESTRICT 
        px     __attribute__(( aligned( 8 ) ));    /* Px/P0 */
        
    SIXTRL_GLOBAL_DEC SIXTRL_REAL_T* SIXTRL_RESTRICT 
        py     __attribute__(( aligned( 8 ) ));    /* Py/P0 */
        
    SIXTRL_GLOBAL_DEC SIXTRL_REAL_T* SIXTRL_RESTRICT 
        sigma  __attribute__(( aligned( 8 ) )); 
            /* s-beta0*c*t  where t is the time
                      since the beginning of the simulation */

    SIXTRL_GLOBAL_DEC SIXTRL_REAL_T* SIXTRL_RESTRICT 
        rpp    __attribute__(( aligned( 8 ) ));    /* ratio P0 /P */
        
    SIXTRL_GLOBAL_DEC SIXTRL_REAL_T* SIXTRL_RESTRICT 
        rvv    __attribute__(( aligned( 8 ) ));    /* ratio beta / beta0 */
        
    SIXTRL_GLOBAL_DEC SIXTRL_INT64_T* SIXTRL_RESTRICT 
        particle_id __attribute__(( aligned( 8 ) ));
    
    NS(block_num_elements_t) num_of_particles  __attribute__(( aligned( 8 ) ));   
} NS(ParticlesSpecial);


NS(ParticlesSpecial)* NS(Blocks_add_particles_special)( 
    NS(Blocks)* SIXTRL_RESTRICT blocks, 
    NS(block_num_elements_t) const num_of_particles )
{
    SIXTRL_GLOBAL_DEC NS(ParticlesSpecial)* ptr_particles = 0;
    
    SIXTRL_STATIC NS(block_size_t) const NUM_ATTR_DATA_POINTERS = 9u;
    SIXTRL_STATIC NS(block_size_t) const REAL_SIZE = sizeof( SIXTRL_REAL_T  );
    SIXTRL_STATIC NS(block_size_t) const I64_SIZE  = sizeof( SIXTRL_INT64_T );
    
    NS(block_size_t) const data_attr_sizes[] =
    {
        REAL_SIZE, REAL_SIZE, REAL_SIZE, REAL_SIZE,
        REAL_SIZE, REAL_SIZE, REAL_SIZE, REAL_SIZE, I64_SIZE
    };
    
    if( ( blocks != 0 ) && 
        ( num_of_particles > ( NS(block_num_elements_t) )0u ) )
    {
        NS(ParticlesSpecial) particles;
        
        NS(block_size_t) const num = ( NS(block_size_t) )num_of_particles;
    
        NS(block_size_t) const data_attr_offsets[] = 
        {
            ( NS(block_size_t) )offsetof( NS(ParticlesSpecial), s ),
            ( NS(block_size_t) )offsetof( NS(ParticlesSpecial), x ),
            ( NS(block_size_t) )offsetof( NS(ParticlesSpecial), y ),
            ( NS(block_size_t) )offsetof( NS(ParticlesSpecial), px ),
            ( NS(block_size_t) )offsetof( NS(ParticlesSpecial), py ),
            ( NS(block_size_t) )offsetof( NS(ParticlesSpecial), sigma ),
            ( NS(block_size_t) )offsetof( NS(ParticlesSpecial), rpp ),
            ( NS(block_size_t) )offsetof( NS(ParticlesSpecial), rvv ),
            ( NS(block_size_t) )offsetof( NS(ParticlesSpecial), particle_id )
        };
        
        NS(block_size_t) const data_attr_counts[] =
        {
            num, num, num, num, num, num, num, num, num
        };
        
        NS(BlockInfo)* ptr_info_block = 0;
        
        particles.s                = NULL;
        particles.x                = NULL;
        particles.y                = NULL;
        particles.px               = NULL;
        particles.py               = NULL;
        particles.sigma            = NULL;
        particles.rpp              = NULL;
        particles.rvv              = NULL;
        particles.particle_id      = NULL;
        particles.num_of_particles = num_of_particles;
        
        ptr_info_block = NS(Blocks_add_block)( blocks, NS(BLOCK_TYPE_PARTICLE), 
                sizeof( particles ), &particles, NUM_ATTR_DATA_POINTERS, 
                data_attr_offsets, data_attr_sizes, data_attr_counts );
        
        if( ptr_info_block != 0 )
        {
            ptr_particles = ( SIXTRL_GLOBAL_DEC NS(ParticlesSpecial)*                     
                )NS(BlockInfo_get_const_ptr_begin)( ptr_info_block );
        }
    }
    
    return ptr_particles;
}

#endif

static const char source[] =
"#if defined(cl_khr_fp64)\n"
"#  pragma OPENCL EXTENSION cl_khr_fp64: enable\n"
"#elif defined(cl_amd_fp64)\n"
"#  pragma OPENCL EXTENSION cl_amd_fp64: enable\n"
"#else\n"
"# error double precision is not supported\n"
"#endif\n"
"#include \"sixtracklib/_impl/namespace_begin.h\"\n"
"#include \"sixtracklib/_impl/definitions.h\"\n"
"#include \"sixtracklib/common/blocks.h\"\n"
"#include \"sixtracklib/common/impl/particles_type.h\"\n"
"#include \"sixtracklib/common/impl/particles_api.h\"\n"
"#include \"sixtracklib/common/particles.h\"\n"
"#include \"sixtracklib/common/impl/beam_elements_type.h\"\n"
"#include \"sixtracklib/common/impl/beam_elements_api.h\"\n"
"#include \"sixtracklib/common/beam_elements.h\"\n"
"kernel void unserialize(\n"
//"       ulong n,\n"
"       global uchar *copy_buffer,\n" // uint8_t is uchar
"       global uchar *copy_buffer_particles,\n" // uint8_t is uchar
"       ulong NUM_PARTICLES\n"
"       )\n"
"{\n"
"    size_t gid = get_global_id(0);\n"
"    NS(Blocks) copied_beam_elements;\n"
"    NS(Blocks_preset)( &copied_beam_elements );\n" // very important for initialization
"    int ret = NS(Blocks_unserialize)(&copied_beam_elements, copy_buffer);\n"
"     NS(Blocks) copied_particles_buffer;\n"
"     NS(Blocks_preset) (&copied_particles_buffer);\n"

"     ret = NS(Blocks_unserialize)(&copied_particles_buffer, copy_buffer_particles);\n"
"}\n\n"


"kernel void track_drift_particle(\n"
"       global uchar *copy_buffer,\n" // uint8_t is uchar
"       global uchar *copy_buffer_particles,\n" // uint8_t is uchar
"       ulong NUM_PARTICLES\n"
"       )\n"
"{\n"
"    size_t gid = get_global_id(0);\n"
"    if(gid >= NUM_PARTICLES) return;\n"
"    NS(block_num_elements_t) ii = gid;\n"
//"    printf(\" | %d\",ii);\n"

// For the particles
"     NS(Blocks) copied_particles_buffer;\n"
"     NS(Blocks_preset) (&copied_particles_buffer);\n"

"     int ret = NS(Blocks_unserialize)(&copied_particles_buffer, copy_buffer_particles);\n"
//"     printf(\"ret = %d\\n\",ret);\n"
"    SIXTRL_GLOBAL_DEC st_BlockInfo const* it  = \n" // is 'it' pointing to the outer particles? check.
"        st_Blocks_get_const_block_infos_begin( &copied_particles_buffer );\n"
"     SIXTRL_GLOBAL_DEC NS(Particles) const* particles = \n"
"            ( SIXTRL_GLOBAL_DEC st_Particles const* )it->begin;\n" 
// *particles now points to the first 'outer' particle
// @ Assuming only a single outer particle
// each 'ii' refers to an inner particle

// for the beam element
"    NS(Blocks) copied_beam_elements;\n"
"    NS(Blocks_preset)( &copied_beam_elements );\n" // very important for initialization
"    ret = NS(Blocks_unserialize)(&copied_beam_elements, copy_buffer);\n"
//"    printf(\"ret = %d\\n\",ret);\n"

"    SIXTRL_GLOBAL_DEC st_BlockInfo const* belem_it  = \n"
"        st_Blocks_get_const_block_infos_begin( &copied_beam_elements );\n"
"   SIXTRL_GLOBAL_DEC st_BlockInfo const* belem_end =\n"
"        st_Blocks_get_const_block_infos_end( &copied_beam_elements );\n"

"    SIXTRL_STATIC SIXTRL_REAL_T const ONE      = ( SIXTRL_REAL_T )1;\n"
"    SIXTRL_STATIC SIXTRL_REAL_T const ONE_HALF = ( SIXTRL_REAL_T )0.5L;\n"
//"     printf(\"%u %u\",(uintptr_t)belem_it,(uintptr_t)belem_end);\n"

// for each particle we apply the beam_elements, as applicable (decided by the switch case)
"    for( ; belem_it != belem_end ; ++belem_it )\n"
"    {\n"
"   st_BlockInfo const info = *belem_it;\n"
"         NS(BlockType) const type_id =  st_BlockInfo_get_type_id(&info );\n"
"        switch( type_id )\n"
"        {\n"
"            case st_BLOCK_TYPE_DRIFT:\n"
"            {\n"
"                __global st_Drift const* drift = \n"
"                    st_Blocks_get_const_drift( &info );\n"
"       st_Drift const drift_private = *drift;"
"          SIXTRL_REAL_T const length = st_Drift_get_length( &drift_private );  \n"
"       SIXTRL_REAL_T const rpp = particles->rpp[ii]; \n"
"       SIXTRL_REAL_T const px = particles->px[ii] * rpp; \n"
"       SIXTRL_REAL_T const py = particles->py[ii] * rpp; \n"
"       SIXTRL_REAL_T const dsigma = \n"
"          ONE - particles->rvv[ii]  * ( ONE + ONE_HALF * ( px * px + py * py ) );\n"
"       SIXTRL_REAL_T sigma = particles->sigma[ii];\n"
"       SIXTRL_REAL_T s = particles->s[ii];\n"
"       SIXTRL_REAL_T x = particles->x[ii];\n"
"       SIXTRL_REAL_T y = particles->y[ii];\n"
"           sigma += length * dsigma;\n"
"           s     += length;\n"
"           x     += length * px;\n"
"           y     += length * py;\n"
"       particles->s[ ii ] = s;\n"
"       particles->x[ ii ] = x;\n"
"       particles->y[ ii ] = y;\n"
"       particles->sigma[ ii ] = sigma;\n"
"                break;\n"
"            }\n"
"            default:\n"
"            {\n"
"                  printf(\"unknown     | --> skipping\\n\");\n"
"            }\n"
"        };\n"
"    }\n"
"      \n"
"}\n";

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
     * we could still add blocks to the buffer. Let's jus do this and 
     * add a different kind of beam element to keep it easier apart! */
    
//    st_DriftExact* drift_exact = st_Blocks_add_drift_exact( 
//        &beam_elements, double{ 0.1 } );
    
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
    
/************************** Preparing grounds for OpenCL *******/
    std::vector<cl::Platform> platform;
    cl::Platform::get(&platform);
    if(platform.empty()) {
        std::cerr << "OpenCL platforms not found." << std::endl;
        return 1;
      }
      else std::cout << "Good" << std::endl;

    // Get all available devices.
    std::vector<cl::Device> devices;
    for(auto p = platform.begin(); devices.empty() && p != platform.end(); p++) {
      std::vector<cl::Device> pldev;
      try {
        p->getDevices(CL_DEVICE_TYPE_ALL, &pldev);
        for(auto d = pldev.begin(); d != pldev.end(); d++) {
          if (!d->getInfo<CL_DEVICE_AVAILABLE>()) continue;
          devices.push_back(*d);
        }
      } catch(...) {
        devices.clear();
      }
    }

    if (devices.empty()) {
      std::cerr << "GPUs with double precision not found." << std::endl;
      return 1;
    }
      // Create context
    cl::Context context;
    context = cl::Context(devices);

//    std::cout << "Device list" << std::endl;
//    for(unsigned int jj=0; jj<devices.size(); jj++){
//      std::cout << "Name of devicei " << jj<<" : "<<devices[jj].getInfo<CL_DEVICE_NAME>() << std::endl;
//      std::cout << "resolution of device timer for device " << jj <<" : "<<devices[jj].getInfo<CL_DEVICE_PROFILING_TIMER_RESOLUTION>() << std::endl;
//    };
/**********************************************/

    /* Let's simulate how to use the serialized data to be passed to 
     * another context - for example to a kernel function in a kernel running 
     * on a GPU or FPGA - where we will then reconstruct the data by 
     * unserializing the buffer */
    
    /* Firstly, copy the buffer to an array large enough to be hold all 
     * the data. Use the range-based constructor of the vector and the 
     * data-range iterators pointing to the "data begin" and "data end" 
     * positions in the serialized buffer*/
    
    std::vector< uint8_t > copy_buffer( 
        st_Blocks_get_const_data_begin( &beam_elements ), 
        st_Blocks_get_const_data_end( &beam_elements ) );
    
    /* this is a completly different buffer, but it contains the same data: */
    

    // launch a kernel with 1 thread and pass the copy_buffer 
    // and call st_Blocks_unserialize on it

    int ndev = 0; // specifying the id of the device to be used
    cl::CommandQueue queue(context, devices[ndev]);
    // Compile OpenCL program for found devices.
    cl::Program program(context, cl::Program::Sources(
        1, std::make_pair(source, strlen(source))
        ));

    try {
    std::string incls = "-D_GPUCODE=1 -D__NAMESPACE=st_ -I" + std::string(NS(PATH_TO_BASE_DIR)) + "include/";
    std::cout << "Path = " << incls << std::endl;
    //program.build(devices, "-D_GPUCODE=1 -D__NAMESPACE=st_ -I/home/sosingh/sixtracklib_gsoc18/initial_test/sixtrack-v0/external/include");
    program.build(devices, incls.c_str());
    } catch (const cl::Error&) {
    std::cerr
      << "OpenCL compilation error" << std::endl
      << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[ndev])
      << std::endl;
    throw;
    }




//    assert( copy_buffer.data() != 
//            st_Blocks_get_const_data_begin( &beam_elements ) );
//    
//    assert( 0 == std::memcmp( copy_buffer.data(), 
//                st_Blocks_get_const_data_begin( &beam_elements ), 
//                copy_buffer.size() ) );
//    
//
//
//    /* Now reconstruct the copied data into a different st_Blocks container */
//    
//    st_Blocks copied_beam_elements;
//    st_Blocks_preset( &copied_beam_elements );
/////    cl::Buffer C(context, CL_MEM_READ_WRITE, sizeof(copied_beam_elements) ); // a separate container
//
//    
//    //TODO: populating the particles struct for each of the particles
//   //       (Not clear how to do it)
//   // Send this to the kernel unserialize as well
//   st_Blocks particles_buffer;
//   st_Blocks_preset( &particles_buffer ); 
//   st_block_size_t const NUM_BLOCKS = 2u; // taken from test_particles.cpp ??
//   st_block_num_elements_t const NUM_PARTICLES = ( st_block_num_elements_t )1000u;
//   st_block_size_t const PARTICLES_DATA_CAPACITY =1048576u;
//   ret = st_Blocks_init(&particles_buffer, NUM_BLOCKS, PARTICLES_DATA_CAPACITY);
//   assert(ret == 0); 
//   st_Particles* particles = st_Blocks_add_particles(&particles_buffer, NUM_PARTICLES );
//   
//   /* This gets rid of the "unused variable" warning/error until you actually use particles */
//   ( void )particles;


  // Allocate device buffers and transfer input data to device.
//  cl::Buffer B(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
//      copy_buffer.size() * sizeof(uint8_t), copy_buffer.data()); // input vector
    cl::Buffer B(context, CL_MEM_READ_WRITE, copy_buffer.size() * sizeof(uint8_t)); // input vector
    queue.enqueueWriteBuffer( B, CL_TRUE, 0, copy_buffer.size() * sizeof(uint8_t), copy_buffer.data() )    ;


//    int numThreads = 1;
//    int blockSize = 1;
//    cl::Kernel unserialize(program, "unserialize");
//    unserialize.setArg(0,B);
//    queue.enqueueNDRangeKernel( 
//    unserialize, cl::NullRange, cl::NDRange( numThreads ), 
//    cl::NDRange(blockSize ));
//    queue.flush();
//    queue.finish();


    // Assuming the particles block and beam_elements block are unserialized on the GPU, we enqueue the kernel track_drift_particle

//    cl::Kernel track_drift_particle(program, "track_drift_particle");
//    numThreads = NUM_OF_BEAM_ELEMENTS; // assign one thread to each bean element
//    blockSize =track_drift_particle.getWorkGroupInfo< CL_KERNEL_WORK_GROUP_SIZE >( devices[ndev]); 
////    track_drift_particle.setArg(0,B);
////    track_drift_particle.setArg(1,D);
//    queue.enqueueNDRangeKernel( 
//    track_drift_particle, cl::NullRange, cl::NDRange( numThreads ), 
//    cl::NDRange(blockSize ));
//    queue.flush();
    

    

#if 0
      // creating a buffer to transfer the data from GPU to CPU

      std::vector< uint8_t > copy_buffer_host(copy_buffer.size());  // output vector

//      cl::Buffer C(context, CL_MEM_READ_WRITE,
//      copy_buffer_host.size() * sizeof(uint8_t) );

      
      queue.enqueueReadBuffer(B, CL_TRUE, 0, copy_buffer_host.size() * sizeof(uint8_t), copy_buffer_host.data());
      queue.flush();

      NS(Blocks) copied_beam_elements_host;
      NS(Blocks_preset)(&copied_beam_elements_host);
      ret = st_Blocks_unserialize( &copied_beam_elements_host, copy_buffer_host.data() );
    
    assert( st_Blocks_get_const_data_begin( &copied_beam_elements_host ) ==
            copy_buffer_host.data() );
    
    assert( st_Blocks_get_num_of_blocks( &copied_beam_elements_host ) ==
            st_Blocks_get_num_of_blocks( &beam_elements ) );
    
    /* Iterate over the copy -> we expect it to have the same data as before */
    
    st_BlockInfo const* copy_it_host  = 
        st_Blocks_get_const_block_infos_begin( &copied_beam_elements_host );
        
    st_BlockInfo const* copy_end_host =
        st_Blocks_get_const_block_infos_end( &copied_beam_elements_host );
        
    std::cout << "\r\n"
              << "Print the beam_elements copied to host: \r\n"
              << "\r\n";
        
    for( ii = 0 ; copy_it_host != copy_end_host ; ++copy_it_host, ++ii )
    {
        std::cout << std::setw( 6 ) << ii << " | type: ";
        
        auto const type_id_host = st_BlockInfo_get_type_id( copy_it_host );
        
        switch( type_id_host )
        {
            case st_BLOCK_TYPE_DRIFT:
            {
                st_Drift const* drift = 
                    st_Blocks_get_const_drift( copy_it_host );
                
                std::cout << "drift        | length = "
                          << std::setw( 10 ) 
                          << st_Drift_get_length( drift )
                          << " [m] \r\n";
                            
                break;
            }
            
            case st_BLOCK_TYPE_DRIFT_EXACT:
            {
                st_DriftExact const* drift_exact =
                    st_Blocks_get_const_drift_exact( copy_it_host );
                
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
    
    std::cout << "\r\n\r\n"
              << "Finished successfully!" << std::endl;
#endif

//    
//    ret = st_Blocks_unserialize( &copied_beam_elements, copy_buffer.data() );
//    
//    /* copied_beam_elements should contain the identical data than 
//     * beam_elements, but the memory addresses should be different -> 
//     * copied_beam_elements use the copy_buffer as storage, 
//     * beam_elements has it's own data stored inside itself */
//    
//    assert( st_Blocks_get_const_data_begin( &copied_beam_elements ) ==
//            copy_buffer.data() );
//    
//    assert( st_Blocks_get_num_of_blocks( &copied_beam_elements ) ==
//            st_Blocks_get_num_of_blocks( &beam_elements ) );
//    
//    /* Iterate over the copy -> we expect it to have the same data as before */
//    
//    st_BlockInfo const* copy_it  = 
//        st_Blocks_get_const_block_infos_begin( &copied_beam_elements );
//        
//    st_BlockInfo const* copy_end =
//        st_Blocks_get_const_block_infos_end( &copied_beam_elements );
//        
//    std::cout << "\r\n"
//              << "Print the copied beam_elements: \r\n"
//              << "\r\n";
//        
//    for( ii = 0 ; copy_it != copy_end ; ++copy_it, ++ii )
//    {
//        std::cout << std::setw( 6 ) << ii << " | type: ";
//        
//        auto const type_id = st_BlockInfo_get_type_id( copy_it );
//        
//        switch( type_id )
//        {
//            case st_BLOCK_TYPE_DRIFT:
//            {
//                st_Drift const* drift = 
//                    st_Blocks_get_const_drift( copy_it );
//                
//                std::cout << "drift        | length = "
//                          << std::setw( 10 ) 
//                          << st_Drift_get_length( drift )
//                          << " [m] \r\n";
//                            
//                break;
//            }
//            
//            case st_BLOCK_TYPE_DRIFT_EXACT:
//            {
//                st_DriftExact const* drift_exact =
//                    st_Blocks_get_const_drift_exact( copy_it );
//                
//                std::cout << "drift_exact  | length = "
//                          << std::setw( 10 )
//                          << st_DriftExact_get_length( drift_exact )
//                          << " [m] \r\n";
//                          
//                break;
//            }
//            
//            default:
//            {
//                std::cout << "unknown     | --> skipping\r\n";
//            }
//        };
//    }
//    
//    std::cout << "\r\n\r\n"
//              << "Finished successfully!" << std::endl;
//    
//    /* Avoiding memory and ressource leaks, we have to clean up after 
//     * ourselves; Since copied_beam_elements uses external storage, we 
//     * do not have to call the st_Blocks_free function on it, but it doesn't
//     * hurt and could be considered safer down the road, as this behaviour 
//     * might change in the future */
//    
//    st_Blocks_free( &beam_elements );
//    st_Blocks_free( &copied_beam_elements );
//    
//    std::cout.flush();

   ////////////////////////// Particles //////////////////////////////// 
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
    
    /* ===================================================================== */
    /* Copy to other buffer to simulate working on the GPU */
    std::cout << "On the GPU:\n"; 
    
    std::vector< uint8_t > copy_buffer_1( 
        st_Blocks_get_const_data_begin( &particles_buffer ), 
        st_Blocks_get_const_data_end( &particles_buffer ) );
    
    
  // Allocate device buffers and transfer input data to device.

    cl::Buffer C(context, CL_MEM_READ_WRITE, copy_buffer_1.size() * sizeof(uint8_t)); // input vector
    queue.enqueueWriteBuffer( C, CL_TRUE, 0, copy_buffer_1.size() * sizeof(uint8_t), copy_buffer_1.data() )    ;

    int numThreads = 1;
    int blockSize = 1;
    cl::Kernel unserialize(program, "unserialize");
    unserialize.setArg(0,B);
    unserialize.setArg(1,C);
    unserialize.setArg(2,NUM_PARTICLES);
    queue.enqueueNDRangeKernel( 
    unserialize, cl::NullRange, cl::NDRange( numThreads ), 
    cl::NDRange(blockSize ));
    queue.flush();
    queue.finish();

//    cl::Kernel unserialize_particle(program, "unserialize_particle");
//    unserialize_particle.setArg(0,C);
//    unserialize_particle.setArg(1,NUM_PARTICLES);
//    queue.enqueueNDRangeKernel( 
//    unserialize_particle, cl::NullRange, cl::NDRange( numThreads ), 
//    cl::NDRange(blockSize ));
//    queue.flush();
//    queue.finish();

    
      // creating a buffer to transfer the data from GPU to CPU

      std::vector< uint8_t > copy_particles_buffer_host(copy_buffer_1.size());  // output vector
    
      queue.enqueueReadBuffer(C, CL_TRUE, 0, copy_particles_buffer_host.size() * sizeof(uint8_t), copy_particles_buffer_host.data());
      queue.flush();

    st_Blocks copy_particles_buffer;
    st_Blocks_preset( &copy_particles_buffer );
    
    ret = st_Blocks_unserialize( &copy_particles_buffer, copy_particles_buffer_host.data() );
    assert( ret == 0 );
    
#if 1
    /* on the GPU, these pointers will have __global as a decorator */

    // On the CPU after copying the data back from the GPU
    std::cout << "\n On the Host, after copying from the GPU; before applying the drift_track_particle mapping\n";
    
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
    
//    st_Blocks_free( &particles_buffer );
//    st_Blocks_free( &copy_particles_buffer );
#endif

// TODO: implement the Track_drift kernel
//       The signature of it will contain something like: (global uchar copy_buffer, global uchar copy_particle_buffer)
//       In the body of the kernel, unserialize the work-item private NS(Block) instance of Particles, beam_elements and then use these instances.

    numThreads = 200;
    blockSize = 100;
    cl::Kernel track_drift_particle(program, "track_drift_particle");
    track_drift_particle.setArg(0,B);
    track_drift_particle.setArg(1,C);
    track_drift_particle.setArg(2,NUM_PARTICLES);
    queue.enqueueNDRangeKernel( 
    track_drift_particle, cl::NullRange, cl::NDRange( numThreads ), 
    cl::NDRange(blockSize ));
    queue.flush();
    queue.finish();

#if 1 
      queue.enqueueReadBuffer(C, CL_TRUE, 0, copy_particles_buffer_host.size() * sizeof(uint8_t), copy_particles_buffer_host.data());
      queue.flush();

    //st_Blocks copy_particles_buffer;
    st_Blocks_preset( &copy_particles_buffer );
    
    ret = st_Blocks_unserialize( &copy_particles_buffer, copy_particles_buffer_host.data() );
    assert( ret == 0 );
    
    /* on the GPU, these pointers will have __global as a decorator */

    // On the CPU after copying the data back from the GPU
    std::cout << "\n On the Host, after applying the drift_track_particles mapping and copying from the GPU\n";
    
    SIXTRL_GLOBAL_DEC st_BlockInfo const* itr  = 
        st_Blocks_get_const_block_infos_begin( &copy_particles_buffer );
    
    SIXTRL_GLOBAL_DEC st_BlockInfo const* endr =
        st_Blocks_get_const_block_infos_end( &copy_particles_buffer );
    
    for( ; itr != endr ; ++itr )
    {
        SIXTRL_GLOBAL_DEC st_Particles const* particles = 
            ( SIXTRL_GLOBAL_DEC st_Particles const* )itr->begin;
            
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
#endif
    return 0;

  }

/* end studies/study8/use_blocks.cpp */
