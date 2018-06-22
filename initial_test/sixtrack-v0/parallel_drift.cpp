/*
* TODO:
* 1. serialize on the CPU -- done
  2. copy data to  the GPU
  3. unserailize (remap) on the GPU -- use a kernel launched with 1 thread to do this
  4. launch a kernel to perform the computation in parallel on the GPU (this is the actual computation)
  5. copy the result back to the cpu 
  6. unserialize the result on the CPUs

  Done!

*/


#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <vector>

#include "sixtracklib/_impl/definitions.h"
#include "sixtracklib/common/blocks.h"
#include "sixtracklib/common/beam_elements.h"
#include "sixtracklib/common/particles.h"

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

static const char source[] =
"#if defined(cl_khr_fp64)\n"
"#  pragma OPENCL EXTENSION cl_khr_fp64: enable\n"
"#elif defined(cl_amd_fp64)\n"
"#  pragma OPENCL EXTENSION cl_amd_fp64: enable\n"
"#else\n"
"# error double precision is not supported\n"
"#endif\n"
//"#include \"../external/include/sixtracklib/_impl/definitions.h\"\n"
//"#include \"../external/include/sixtracklib/common/blocks.h\"\n"
//"#include \"../external/include/sixtracklib/common/beam_elements.h\"\n"
//"#include \"../external/include/sixtracklib/common/particles.h\"\n"
"kernel void unserialize(\n"
//"       ulong n,\n"
"       global uchar *copy_buffer\n" // uint8_t is uchar
//"       global double *c\n"
"       )\n"
"{\n"
"    size_t gid = get_global_id(0);\n"
"    printf(\"Hello from GPU\");\n"
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
    
    st_DriftExact* drift_exact = st_Blocks_add_drift_exact( 
        &beam_elements, double{ 0.1 } );
    
    assert( drift_exact != nullptr );
    
    assert( st_Blocks_get_num_of_blocks( &beam_elements ) == 
            ( NUM_OF_BEAM_ELEMENTS + 1 ) );
    
    /* Always safely terminate pointer variables pointing to resources they
     * do not own which we no longer need -> just a good practice */
    
    drift_exact = nullptr;
    
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
    

    // Allocate device buffers and transfer input data to device.
    cl::Buffer B(context, CL_MEM_READ_WRITE, copy_buffer.size() * sizeof(uint8_t)); // input vector
    // launch a kernel with 1 thread and pass the copy_buffer 
    // and call st_Blocks_unserialize on it

    int ndev = 0; // specifying the id of the device to be used
    cl::CommandQueue queue(context, devices[ndev]);
    // Compile OpenCL program for found devices.
    cl::Program program(context, cl::Program::Sources(
        1, std::make_pair(source, strlen(source))
        ));

    try {
    program.build(devices);
    } catch (const cl::Error&) {
    std::cerr
      << "OpenCL compilation error" << std::endl
      << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[ndev])
      << std::endl;
    throw;
    }

    int numThreads = 1;
    int blockSize = 1;
    cl::Kernel unserialize(program, "unserialize");
    unserialize.setArg(0,B);
    queue.enqueueNDRangeKernel( 
    unserialize, cl::NullRange, cl::NDRange( numThreads ), 
    cl::NDRange(blockSize ));
    queue.flush();



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


    return 0;

  }

/* end studies/study8/use_blocks.cpp */
