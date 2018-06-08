// This code finds the sum of the elements of a vector 
//NOTE: The reduce kernel expects the blockSize to be a "power of 2" to work correctly. Modify the code so that it works for any blockSize
// Profiling the code 

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

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
"kernel void reduce(\n"
"       ulong n,\n"
"       global double *b,\n"
"       global double *c\n"
"       )\n"
"{\n"
"    size_t gid = get_global_id(0);\n"
"    size_t tid = get_local_id(0);\n"
"  // do reduction in global mem\n"
"  for (unsigned int s = get_local_size(0) / 2; s > 0; s >>= 1)\n"
"  {\n"
"    if (tid < s)\n"
"  {\n"
"      b[gid] += b[gid + s];\n"
"    }\n"
"   barrier(CLK_LOCAL_MEM_FENCE);// make sure all adds at one stage are done!\n"
"    }\n"

"  // only thread 0 writes result for this work-group back to global mem\n"
"  if (tid == 0)\n"
"  {\n"
"    c[get_group_id(0)] = b[gid];\n"
"  }\n"

"}\n";

int mk_test(std::vector<cl::Device> devices, int ndev,  cl::Context context, 
            std::size_t const N ) 
{
    //std::cout << "Using " << devices[ndev].getInfo<CL_DEVICE_NAME>() << std::endl;

    // Create command queue.
    cl::CommandQueue queue(context, devices[ndev],CL_QUEUE_PROFILING_ENABLE);
    // the third parameer is to enable profiling

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

    cl::Kernel reduce(program, "reduce");
    //size_t N = 1 << 20;
    //size_t N = 3225; // N accepted as a command line argument
    std::cout << "N = " << N << std::endl;
    #if 0
    // Prepare input data.
    const size_t blockSize = 1024; // # of threads per work-group (set by default is 256)
    #endif  
    /* ----------------------------------------------------------------------- */
    /* ---- Find the "optimal" work-group size:
    * ---- In this first iteration, we try to use the largest number of 
    * ---- threads per work-group available;
    * ----------------------------------------------------------------------- */
    #if 1 
    // determine the work-group size
    size_t blockSize = 
    reduce.getWorkGroupInfo< CL_KERNEL_WORK_GROUP_SIZE >( devices[ndev]); 

    /* Any OpenCL Device has a number of computing units. At best, 
    * blockSize * work_group_size number of threads are active at a time; */

    size_t const maxNumOfComputeUnits = 
    devices[ndev].getInfo< CL_DEVICE_MAX_COMPUTE_UNITS >();

    /* For the given kernel + device, a work-group size in multiples of 
    * this number is preferred */

    size_t const preferredWorkGroupFactor = 
    reduce.getWorkGroupInfo< CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>( 
        devices[ ndev ] );
    #endif
    assert((blockSize & (blockSize-1)) == 0); // ensure blockSize is a power of 2
    size_t numBlocks = (N+blockSize-1)/blockSize; // the ceil of N/blockSize
    size_t N_sz = numBlocks * blockSize; // The multiple of blockSize greater than N and closest to N
    //std::cout << "N_sz = " << N_sz << std::endl;
    std::vector<double> b(N_sz, 0); 
    std::vector<double> c(numBlocks);

    //  std::vector<double> c(numBlocks, 0.0);

    double average_time = 0.0;
    double num_of_turns = 0.0;

    std::size_t const NUM_REPETITIONS = 1010;

    assert( N_sz == b.size() );
    assert( numBlocks == c.size() );

    std::size_t const NUM_BYTES_B = sizeof( double ) * N_sz;
    std::size_t const NUM_BYTES_C = sizeof( double ) * numBlocks;

    // Allocate device buffers and transfer input data to device.
    cl::Buffer B(context, CL_MEM_READ_WRITE, b.size() * sizeof(double)); // input vector
    cl::Buffer C(context, CL_MEM_READ_WRITE, c.size() * sizeof(double)); // output vector

    reduce.setArg(0, static_cast<cl_ulong>(N));
    reduce.setArg(1, B);
    reduce.setArg(2, C);

    for( std::size_t ll = 0 ; ll < NUM_REPETITIONS ; ++ll )
    {
        auto b_end_of_data = b.begin();
        std::advance( b_end_of_data, N );

        std::fill( b.begin(), b_end_of_data, static_cast< double >( ll ) );
        std::fill( b_end_of_data, b.end(), 0.0 );
        std::fill( c.begin(), c.end(), 0.0 );

        cl::Event event;

        cl_int ret = CL_SUCCESS;

        ret  = queue.enqueueWriteBuffer( C, CL_TRUE, 0, NUM_BYTES_C, c.data() );
        ret |= queue.enqueueWriteBuffer( B, CL_TRUE, 0, NUM_BYTES_B, b.data() );      

        ret |= queue.enqueueNDRangeKernel( 
              reduce, cl::NullRange, cl::NDRange( N_sz ), 
              cl::NDRange( blockSize ), nullptr, &event );

        queue.flush();

        assert( ret == CL_SUCCESS );
        event.wait();

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

        assert( ret == CL_SUCCESS );

        ret  = queue.enqueueReadBuffer( C, CL_TRUE, 0, NUM_BYTES_C, c.data() );
        /* enqueueReadBuffer() is an implicit queue.flush() */

        assert( ret == CL_SUCCESS );

        double const kernel_time_elapsed = when_kernel_ended - when_kernel_started;

        if( ll > 10u )
        {
            num_of_turns += 1.0;
            average_time += (kernel_time_elapsed-average_time)/num_of_turns; // finding the running average
        }

        double sum_reduce = 0.0;

        for( auto const& partial_sum : c ) // finding the block-wise sum on the host
        {
            sum_reduce += partial_sum;
        }

//        std::cout << "ll = " << std::setw( 5 ) << ll 
//                  << " :: sum_reduce = " << sum_reduce << std::endl;
    }
    
//    if( num_of_turns > 1.0 )
//    {
//        average_time /= num_of_turns;
//    }
    
    std::cout << "average time taken = " << average_time << std::endl;
    
    return 0;
}

int main(int argc, char *argv[]) {

  if(argc != 2) {
      std::cerr << "Usage: " << argv[0] << " <size of the input>" << std::endl;
      exit(1);
    }
  size_t N = atoi(argv[1]); // getting the size of the input

  try {
    // Get list of OpenCL platforms.
    std::vector<cl::Platform> platform;
    cl::Platform::get(&platform);

    if (platform.empty()) {
      std::cerr << "OpenCL platforms not found." << std::endl;
      return 1;
    }

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

    std::cout << "Device list" << std::endl;
    for(int jj=0; jj<devices.size(); jj++){
      std::cout << "Name of devicei " << jj<<" : "<<devices[jj].getInfo<CL_DEVICE_NAME>() << std::endl;
      std::cout << "resolution of device timer for device " << jj <<" : "<<devices[jj].getInfo<CL_DEVICE_PROFILING_TIMER_RESOLUTION>() << std::endl;
      mk_test(devices,jj,context,N);
    };
    return 0;

  } catch (const cl::Error &err) {
    std::cerr
      << "OpenCL error: "
      << err.what() << "(" << err.err() << ")"
      << std::endl;
    return 1;
  }
}
