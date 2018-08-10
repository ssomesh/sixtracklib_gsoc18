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
**************************************************
Only drift and drift_exact have the same amount of space.
Multipole for example has additonal data-members and takes more space.
***************************************************
*/

// The kernel file is "kernels_beam_elements_oneatatime.cl"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <random>
#include <vector>
#include <iterator>
#include <fstream>

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

int main(int argc, char** argv)
{
      if(argc < 4) {
          std::cerr << "Usage: " << argv[0] << " < #particles > < #turns > < tracking_function_id > [deviceIdx]" << std::endl;
          exit(1);
        }
  		int NUM_REPETITIONS = 10;//1010; // for benchmarking
    	double num_of_turns = 0.0; // for timing
    	double average_execution_time_drift = 0.0;
    	double average_execution_time_drift_exact = 0.0;
    	double average_execution_time_cavity = 0.0;
    	double average_execution_time_align = 0.0;

      std::vector<double> exec_time_drift;
      std::vector<double> exec_time_drift_exact;
      std::vector<double> exec_time_cavity;
      std::vector<double> exec_time_align;

			for(int ll = 0; ll < NUM_REPETITIONS; ++ll) {
    /* We will use 9+ beam element blocks in this example and do not
     * care to be memory efficient yet; thus we make the blocks for
     * beam elements and particles big enough to avoid running into problems */

    constexpr st_block_size_t const MAX_NUM_BEAM_ELEMENTS       = 1000u; // 20u;
    constexpr st_block_size_t const NUM_OF_BEAM_ELEMENTS        = 1000u; //9u;

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

   // One-fourth of the beam-elements are drift-elements
    for( st_block_size_t ii = 0 ; ii < NUM_OF_BEAM_ELEMENTS/4 ; ++ii )
    {
        double const drift_length = double{ 0.2L };
        st_Drift* drift = st_Blocks_add_drift( &beam_elements, drift_length );

        (void)drift; // using the variable with a no-op

        assert( drift != nullptr ); /* Otherwise, there was a problem! */
    }

    /* Check if we *really* have the correct number of beam elements and
     * if they really are all drifts */

    assert( st_Blocks_get_num_of_blocks( &beam_elements ) ==
            NUM_OF_BEAM_ELEMENTS/4 );

    /* The beam_elements container is currently not serialized yet ->
     * we could still add blocks to the buffer. Let's jus do this and
     * add a different kind of beam element to keep it easier apart! */

    for( st_block_size_t ii = NUM_OF_BEAM_ELEMENTS/4 ; ii < NUM_OF_BEAM_ELEMENTS/2 ; ++ii )
    {
        double const drift_length = double{ 0.1L };
    st_DriftExact* drift_exact = st_Blocks_add_drift_exact(
        &beam_elements, drift_length );
        (void) drift_exact;
    assert( drift_exact != nullptr );
   }

    assert( st_Blocks_get_num_of_blocks( &beam_elements ) ==
            ( NUM_OF_BEAM_ELEMENTS*0.5) );

    /* Adding the beam element 'cavity' */

    for( st_block_size_t ii = NUM_OF_BEAM_ELEMENTS*0.5 ; ii < NUM_OF_BEAM_ELEMENTS*0.75 ; ++ii )
    {
      double const voltage = double{ 1e4};
      double const frequency = double{ 40};
      double const lag = double{ 0.01L};
      st_Cavity* cavity = st_Blocks_add_cavity(
          &beam_elements, voltage, frequency, lag);
      (void) cavity; // a no-op
      assert( cavity != nullptr ); /* Otherwise, there was a problem! */
    }
    assert( st_Blocks_get_num_of_blocks( &beam_elements ) ==
            ( NUM_OF_BEAM_ELEMENTS * 0.75) );

    /* Adding the beam element 'align' */
    double const M__PI   = // note the two underscores between M and PI
      ( double )3.1415926535897932384626433832795028841971693993751L;
    for( st_block_size_t ii = NUM_OF_BEAM_ELEMENTS*0.75 ; ii < NUM_OF_BEAM_ELEMENTS ; ++ii )
    {
      double const tilt = double{ 0.5};
      double const z = double{ M__PI / 45};
      double const dx = double{ 0.2L};
      double const dy = double{ 0.2L};
      st_Align* align = st_Blocks_add_align(
          &beam_elements, tilt, cos( z ), sin( z ), dx, dy);
      (void) align; // a no-op
      assert( align != nullptr ); /* Otherwise, there was a problem! */
    }
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
#if 0
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
#endif
    std::cout.flush();

/************************** Preparing grounds for OpenCL *******/
    std::vector<cl::Platform> platform;
    cl::Platform::get(&platform);

    if( platform.empty() )
    {
        std::cerr << "OpenCL platforms not found." << std::endl;
        return 1;
    }

    std::vector< cl::Device > devices;

    for( auto const& p : platform )
    {
        std::vector< cl::Device > temp_devices;

        p.getDevices( CL_DEVICE_TYPE_ALL, &temp_devices );

        for( auto const& d : temp_devices )
        {
            if( !d.getInfo< CL_DEVICE_AVAILABLE >() ) continue;
            devices.push_back( d );
        }
    }

    cl::Device* ptr_selected_device = nullptr;

    if( !devices.empty() )
    {
        if( argc >= 5 )
        {
            std::size_t const device_idx = std::atoi( argv[ 4 ] );

            if( device_idx < devices.size() )
            {
                ptr_selected_device = &devices[ device_idx ];
            }
        }

        if( ptr_selected_device == nullptr )
        {
            std::cout << "default selecting device #0" << std::endl;
            ptr_selected_device = &devices[ 0 ];
        }
    }

    if( ptr_selected_device != nullptr )
    {
        std::cout << "device: "
                  << ptr_selected_device->getInfo< CL_DEVICE_NAME >()
                  << std::endl;
    }
    else return 0;

    cl::Context context( *ptr_selected_device );

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

//    std::vector< uint8_t > copy_buffer(
//        st_Blocks_get_const_data_begin( &beam_elements ),
//        st_Blocks_get_const_data_end( &beam_elements ) );

    /* this is a completly different buffer, but it contains the same data: */


    // launch a kernel with 1 thread and pass the copy_buffer
    // and call st_Blocks_unserialize on it

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // getting the kernel file
   std::string PATH_TO_KERNEL_FILE( st_PATH_TO_BASE_DIR );
       PATH_TO_KERNEL_FILE += "../kernels_beam_elements_oneatatime.cl";

       std::string kernel_source( "" );
       std::ifstream kernel_file( PATH_TO_KERNEL_FILE.c_str(),
                                  std::ios::in | std::ios::binary );

       if( kernel_file.is_open() )
       {
           std::istreambuf_iterator< char > file_begin( kernel_file.rdbuf() );
           std::istreambuf_iterator< char > end_of_file;

           kernel_source.assign( file_begin, end_of_file );
           kernel_file.close();
       }
////////////////////////////////////////////////////////////////////////////////////////////////////////////
    assert( ptr_selected_device != nullptr );

  //  int ndev = 0; // specifying the id of the device to be used
    cl::CommandQueue queue(context, *ptr_selected_device,CL_QUEUE_PROFILING_ENABLE);
    // Compile OpenCL program for found devices.
			cl:: Program program(context, kernel_source); //string  kernel_source contains the kernel(s) read from the file

#if 0
/////////////////////// Alternative 1 for including the kernels written in a separate file -- works perfectly fine /////////////////////////////////
			cl:: Program program(context, "#include \"../kernels.cl\" ", false); // the path inside the #include should be relative to an include directory specified using -Ipath/to/dir specified via build options.. otherwise give the absolute path.
#endif

#if 0
/////////////////////// The way to go if the string source[] contains the source in the same file as this.

//    cl::Program program(context, cl::Program::Sources(
//        1, std::make_pair(source, strlen(source))
//        ));
#endif


    try {
    std::string incls = "-D_GPUCODE=1 -D__NAMESPACE=st_ -I" + std::string(NS(PATH_TO_BASE_DIR)) + "include/";
  //  std::cout << "Path = " << incls << std::endl;
    //program.build(devices, "-D_GPUCODE=1 -D__NAMESPACE=st_ -I/home/sosingh/sixtracklib_gsoc18/initial_test/sixtrack-v0/external/include");
    program.build( incls.c_str() );
    } catch (const cl::Error&) {
    std::cerr
      << "OpenCL compilation error" << std::endl
      << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(*ptr_selected_device)
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


//   const int QUARTER_NUM_OF_BEAM_ELEMENTS = (int)NUM_OF_BEAM_ELEMENTS/4;
  // Allocate device buffers and transfer input data to device.
//  cl::Buffer B(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
//      copy_buffer.size() * sizeof(uint8_t), copy_buffer.data()); // input vector
    cl::Buffer B(context, CL_MEM_READ_WRITE, st_Blocks_get_total_num_bytes( &beam_elements  )); // input vector
 queue.enqueueWriteBuffer( B, CL_TRUE, 0, st_Blocks_get_total_num_bytes( &beam_elements ), st_Blocks_get_const_data_begin( &beam_elements ) );

#if 0
    cl::Buffer B(context, CL_MEM_READ_WRITE, st_Blocks_get_total_num_bytes( &beam_elements )/4); // input vector for drift
    cl::Buffer D(context, CL_MEM_READ_WRITE, st_Blocks_get_total_num_bytes( &beam_elements )/4); // input vector for drift_exact
    cl::Buffer E(context, CL_MEM_READ_WRITE, st_Blocks_get_total_num_bytes( &beam_elements )/4); // input vector for cavity
    cl::Buffer F(context, CL_MEM_READ_WRITE, st_Blocks_get_total_num_bytes( &beam_elements )/4); // input vector for align

queue.enqueueWriteBuffer( B, CL_TRUE, 0, st_Blocks_get_total_num_bytes( &beam_elements )/4, st_Blocks_get_const_data_begin( &beam_elements ) );
queue.enqueueWriteBuffer( D, CL_TRUE, 0, st_Blocks_get_total_num_bytes( &beam_elements )/4, st_Blocks_get_const_data_begin( &beam_elements)+(QUARTER_NUM_OF_BEAM_ELEMENTS ) );
queue.enqueueWriteBuffer( E, CL_TRUE, 0, st_Blocks_get_total_num_bytes( &beam_elements )/4, st_Blocks_get_const_data_begin( &beam_elements)+(2*QUARTER_NUM_OF_BEAM_ELEMENTS ) );
queue.enqueueWriteBuffer( F, CL_TRUE, 0, st_Blocks_get_total_num_bytes( &beam_elements )/4, st_Blocks_get_const_data_begin( &beam_elements)+(3*QUARTER_NUM_OF_BEAM_ELEMENTS)) ;
#endif
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
    st_block_size_t const PARTICLES_DATA_CAPACITY = 1048576u*1000*4; //  ~(4 GB)
    st_block_size_t const NUM_PARTICLES           = atoi(argv[1]); // 100u;

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

        assert( particles->num_of_particles == (int)NUM_PARTICLES );

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
    //std::cout << "On the GPU:\n";

  // Allocate device buffers and transfer input data to device.

    cl::Buffer C(context, CL_MEM_READ_WRITE, st_Blocks_get_total_num_bytes( &particles_buffer )); // input vector
		queue.enqueueWriteBuffer( C, CL_TRUE, 0, st_Blocks_get_total_num_bytes( &particles_buffer ), st_Blocks_get_const_data_begin( &particles_buffer ) );

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



      // creating a buffer to transfer the data from GPU to CPU

      std::vector< uint8_t > copy_particles_buffer_host(st_Blocks_get_total_num_bytes( &particles_buffer )/sizeof(uint8_t));  // output vector

      queue.enqueueReadBuffer(C, CL_TRUE, 0, copy_particles_buffer_host.size() * sizeof(uint8_t), copy_particles_buffer_host.data());
      queue.flush();

    st_Blocks copy_particles_buffer;
    st_Blocks_preset( &copy_particles_buffer );

    ret = st_Blocks_unserialize( &copy_particles_buffer, copy_particles_buffer_host.data() );
    assert( ret == 0 );

#if 0
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

    SIXTRL_UINT64_T const NUM_TURNS = atoi(argv[2]);//100;
    SIXTRL_UINT64_T offset = 0;
    cl::Event event;

  switch (atoi(argv[3]))
  {
    case 1 :
    {
    cl::Event event;
    cl::Kernel track_drift_particle(program, "track_drift_particle");
    blockSize = track_drift_particle.getWorkGroupInfo< CL_KERNEL_WORK_GROUP_SIZE >( *ptr_selected_device);// determine the work-group size
    numThreads = ((NUM_PARTICLES+blockSize-1)/blockSize) * blockSize; // rounding off NUM_PARTICLES to the next nearest multiple of blockSize. This is to ensure that there are integer number of work-groups launched
    std::cout << blockSize << " " << numThreads<< std::endl;
    track_drift_particle.setArg(0,B);
    track_drift_particle.setArg(1,C);
    track_drift_particle.setArg(2,NUM_PARTICLES);
    track_drift_particle.setArg(3,NUM_TURNS);
    track_drift_particle.setArg(4,offset);



    queue.enqueueNDRangeKernel(
    track_drift_particle, cl::NullRange, cl::NDRange( numThreads ),
    cl::NDRange(blockSize ), nullptr, &event);
    queue.flush();
    event.wait();
    queue.finish();

        cl_ulong when_kernel_queued    = 0;
        cl_ulong when_kernel_submitted = 0;
        cl_ulong when_kernel_started   = 0;
        cl_ulong when_kernel_ended     = 0;

        ret  = event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_QUEUED, &when_kernel_queued );

        ret |= event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_SUBMIT, &when_kernel_submitted );

        ret |= event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_START, &when_kernel_started );

        ret |= event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_END, &when_kernel_ended );

        assert( ret == CL_SUCCESS ); // all ret's should be 1

        double const kernel_time_elapsed = when_kernel_ended - when_kernel_started;
        exec_time_drift.push_back(kernel_time_elapsed);
        if( ll > 5 ) {
          num_of_turns += 1.0;
          average_execution_time_drift += (kernel_time_elapsed - average_execution_time_drift)/num_of_turns;
      }
      break;
    }
    case 2:
    {

    offset = 250;
    cl::Event event;
    cl::Kernel track_drift_exact_particle(program, "track_drift_exact_particle");
    blockSize = track_drift_exact_particle.getWorkGroupInfo< CL_KERNEL_WORK_GROUP_SIZE >( *ptr_selected_device);// determine the work-group size
    numThreads = ((NUM_PARTICLES+blockSize-1)/blockSize) * blockSize; // rounding off NUM_PARTICLES to the next nearest multiple of blockSize. This is to ensure that there are integer number of work-groups launched
    std::cout << blockSize << " " << numThreads<< std::endl;
    track_drift_exact_particle.setArg(0,B);
    track_drift_exact_particle.setArg(1,C);
    track_drift_exact_particle.setArg(2,NUM_PARTICLES);
    track_drift_exact_particle.setArg(3,NUM_TURNS);
    track_drift_exact_particle.setArg(4,offset);
    queue.enqueueNDRangeKernel(
    track_drift_exact_particle, cl::NullRange, cl::NDRange( numThreads ),
    cl::NDRange(blockSize ), nullptr, &event);
    queue.flush();
    event.wait();
    queue.finish();
        cl_ulong when_kernel_queued    = 0;
        cl_ulong when_kernel_submitted = 0;
        cl_ulong when_kernel_started   = 0;
        cl_ulong when_kernel_ended     = 0;

        ret  = event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_QUEUED, &when_kernel_queued );

        ret |= event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_SUBMIT, &when_kernel_submitted );

        ret |= event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_START, &when_kernel_started );

        ret |= event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_END, &when_kernel_ended );

        assert( ret == CL_SUCCESS ); // all ret's should be 1

        double const kernel_time_elapsed = when_kernel_ended - when_kernel_started;
        exec_time_drift_exact.push_back(kernel_time_elapsed);
        if( ll > 5 ) {
          num_of_turns += 1.0;
          average_execution_time_drift_exact += (kernel_time_elapsed - average_execution_time_drift_exact)/num_of_turns;
      }
    break;
    }
    case 3:
    {
    offset = 500;
    cl::Event event;
    cl::Kernel track_cavity_particle(program, "track_cavity_particle");
    blockSize = track_cavity_particle.getWorkGroupInfo< CL_KERNEL_WORK_GROUP_SIZE >( *ptr_selected_device);// determine the work-group size
    numThreads = ((NUM_PARTICLES+blockSize-1)/blockSize) * blockSize; // rounding off NUM_PARTICLES to the next nearest multiple of blockSize. This is to ensure that there are integer number of work-groups launched
    std::cout << blockSize << " " << numThreads<< std::endl;
    track_cavity_particle.setArg(0,B);
    track_cavity_particle.setArg(1,C);
    track_cavity_particle.setArg(2,NUM_PARTICLES);
    track_cavity_particle.setArg(3,NUM_TURNS);
    track_cavity_particle.setArg(4,offset);
    queue.enqueueNDRangeKernel(
    track_cavity_particle, cl::NullRange, cl::NDRange( numThreads ),
    cl::NDRange(blockSize ), nullptr, &event);
    queue.flush();
    event.wait();
    queue.finish();
        cl_ulong when_kernel_queued    = 0;
        cl_ulong when_kernel_submitted = 0;
        cl_ulong when_kernel_started   = 0;
        cl_ulong when_kernel_ended     = 0;

        ret  = event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_QUEUED, &when_kernel_queued );

        ret |= event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_SUBMIT, &when_kernel_submitted );

        ret |= event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_START, &when_kernel_started );

        ret |= event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_END, &when_kernel_ended );

        assert( ret == CL_SUCCESS ); // all ret's should be 1

        double const kernel_time_elapsed = when_kernel_ended - when_kernel_started;
        exec_time_cavity.push_back(kernel_time_elapsed);
        if( ll > 5 ) {
          num_of_turns += 1.0;
          average_execution_time_cavity += (kernel_time_elapsed - average_execution_time_cavity)/num_of_turns;
      }
    break;
    }
    case 4:
    {
    cl::Event event;
    offset = 750;
    cl::Kernel track_align_particle(program, "track_align_particle");
    blockSize = track_align_particle.getWorkGroupInfo< CL_KERNEL_WORK_GROUP_SIZE >( *ptr_selected_device);// determine the work-group size
    numThreads = ((NUM_PARTICLES+blockSize-1)/blockSize) * blockSize; // rounding off NUM_PARTICLES to the next nearest multiple of blockSize. This is to ensure that there are integer number of work-groups launched
    std::cout << blockSize << " " << numThreads<< std::endl;
    track_align_particle.setArg(0,B);
    track_align_particle.setArg(1,C);
    track_align_particle.setArg(2,NUM_PARTICLES);
    track_align_particle.setArg(3,NUM_TURNS);
    track_align_particle.setArg(4,offset);
    queue.enqueueNDRangeKernel(
    track_align_particle, cl::NullRange, cl::NDRange( numThreads ),
    cl::NDRange(blockSize ), nullptr, &event);
    queue.flush();
    event.wait();
    queue.finish();
        cl_ulong when_kernel_queued    = 0;
        cl_ulong when_kernel_submitted = 0;
        cl_ulong when_kernel_started   = 0;
        cl_ulong when_kernel_ended     = 0;

        ret  = event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_QUEUED, &when_kernel_queued );

        ret |= event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_SUBMIT, &when_kernel_submitted );

        ret |= event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_START, &when_kernel_started );

        ret |= event.getProfilingInfo< cl_ulong >(
          CL_PROFILING_COMMAND_END, &when_kernel_ended );

        assert( ret == CL_SUCCESS ); // all ret's should be 1

        double const kernel_time_elapsed = when_kernel_ended - when_kernel_started;
        exec_time_align.push_back(kernel_time_elapsed);
        if( ll > 5 ) {
          num_of_turns += 1.0;
          average_execution_time_align += (kernel_time_elapsed - average_execution_time_align)/num_of_turns;
      }
    break;
    }
 }; // end of switch case
      queue.enqueueReadBuffer(C, CL_TRUE, 0, copy_particles_buffer_host.size() * sizeof(uint8_t), copy_particles_buffer_host.data());
      queue.flush();

    //st_Blocks copy_particles_buffer;
    st_Blocks_preset( &copy_particles_buffer );

    ret = st_Blocks_unserialize( &copy_particles_buffer, copy_particles_buffer_host.data() );
    assert( ret == 0 );

    /* on the GPU, these pointers will have __global as a decorator */

#if 0
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

#endif
    std::cout.flush();
    st_Blocks_free( &particles_buffer );
    st_Blocks_free( &copy_particles_buffer );
  } // end of the NUM_REPETITIONS 'for' loop
  switch(atoi(argv[3]))
  {
    case 1:
    {
      // printing the contents of the exec_time vector
    for(std::vector<double>::iterator it = exec_time_drift.begin(); it != exec_time_drift.end(); ++it)
      printf("%.3f s%c",(*it)*1.0e-9, ",\n"[it+1 == exec_time_drift.end()]);
		printf("Reference Version: Time = %.3f s; \n",average_execution_time_drift*1.0e-9);
    break;
    }
    case 2:
    {
    for(std::vector<double>::iterator it = exec_time_drift_exact.begin(); it != exec_time_drift_exact.end(); ++it)
      printf("%.3f s%c",(*it)*1.0e-9, ",\n"[it+1 == exec_time_drift_exact.end()]);
		printf("Reference Version: Time = %.3f s; \n",average_execution_time_drift_exact*1.0e-9);
    break;
    }
    case 3:
    {
    for(std::vector<double>::iterator it = exec_time_cavity.begin(); it != exec_time_cavity.end(); ++it)
      printf("%.3f s%c",(*it)*1.0e-9, ",\n"[it+1 == exec_time_cavity.end()]);
		printf("Reference Version: Time = %.3f s; \n",average_execution_time_cavity*1.0e-9);
    break;
    }
    case 4:
    {
    for(std::vector<double>::iterator it = exec_time_align.begin(); it != exec_time_align.end(); ++it)
      printf("%.3f s%c",(*it)*1.0e-9, ",\n"[it+1 == exec_time_align.end()]);
		printf("Reference Version: Time = %.3f s; \n",average_execution_time_align*1.0e-9);
    break;
    }
  };
    return 0;

  }

/* end studies/study8/use_blocks.cpp */
