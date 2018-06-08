// This code finds the sum of the elements of a vector 
// local memory is used -- declared externally
//FIXME: The reduce kernel expects the blockSize to be a "power of 2" to work correctly. Modify the code so that it works for any blockSize

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sys/time.h>

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
"       global double *c, \n"
"       local double* shm )\n"
"{\n"
"    size_t gid = get_global_id(0);\n"
"    size_t tid = get_local_id(0);\n"
"    // Read data to local memory\n"
"    shm[tid]  = ( gid < n ) ? b[gid] : 0.0;\n"   
"    barrier(CLK_LOCAL_MEM_FENCE);\n // ensure entire block is loaded to local memory" 
"    // do reduction in local mem\n"
"    for (unsigned int s = get_local_size(0) / 2; s > 0; s >>= 1)\n"
"    {\n"
"      if (tid < s)\n"
"      {\n"
"         shm[tid] += shm[tid + s];\n"
"      }\n"
"      barrier(CLK_LOCAL_MEM_FENCE);// make sure all adds at one stage are done!\n"
"    }\n"
"    // only thread 0 writes result for this work-group back to global mem\n"

"    if (tid == 0)\n"
"    {\n"
"       c[get_group_id(0)] = shm[0];\n"
"     }\n"
"}\n";


double rtclock()
{
  struct timezone Tzp;
  struct timeval Tp;
  uint64_t stat;
  stat = gettimeofday (&Tp, &Tzp);
  if (stat != 0) printf("Error return from gettimeofday: %d",stat);
  return(Tp.tv_sec + Tp.tv_usec*1.0e-6);
}



int mk_test(std::vector<cl::Device> devices, int ndev,  cl::Context context, size_t N) {
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
  //size_t N = 3225; 
  std::cout << "N = " << N << std::endl;
#if 0
  // Prepare input data.
  // Manually specifying the blocksize
  const size_t blockSize = 1024; // # of threads per work-group (set by default is 256)
#endif  
  
  /* ----------------------------------------------------------------------- */
  /* ---- Find the "optimal" work-group size:
   * ---- In this first iteration, we try to use the largest number of 
   * ---- threads per work-group available;
   * ----------------------------------------------------------------------- */
  
#if 1 

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
  
  /* This is the max amount of local memory per compute unit(!) 
   * ( == MP in CUDA) */
  /* guaranteed by the standard to be >= 16KByte */
  
  size_t const maxLocalMemSize = 
      devices[ndev].getInfo< CL_DEVICE_LOCAL_MEM_SIZE >();
   
  std::cout << "maxLocalMemSize: " << maxLocalMemSize << " Bytes" << std::endl;
      
  if( maxLocalMemSize < sizeof( double ) * blockSize )
  {
      size_t maxNumOfThreadsByMem = maxLocalMemSize / sizeof( double );
      assert( maxNumOfThreadsByMem != 0u );
      
      size_t const nn = maxNumOfThreadsByMem / preferredWorkGroupFactor;
      blockSize = nn * preferredWorkGroupFactor;
      
  }
  
  assert( blockSize != 0u ); // to avoid division by 0 error
  assert((blockSize & (blockSize-1)) == 0); // ensure blockSize is a power of 2
  
  size_t numBlocks = (N+blockSize-1)/blockSize; // the ceil of N/blockSize
  
 //  assert( numBlocks < maxNumOfComputeUnits ); // why is this required? I think it is not required. This is required if we want all blocks to be resident, otherwise not
      
  size_t N_sz = numBlocks * blockSize; // The multiple of blockSize greater than N and closest to N
  //std::cout << "N_sz = " << N_sz << std::endl;
  
  std::vector<double> b(N, 0); 
  std::vector<double> c(numBlocks);

  for (int jj=0; jj<N; jj++)
  {
    b[jj]=1.0;//2.0 * jj;
  };
 // The index [N,N_sz-1] are padded with zeros; in general it can be padded with the identity element for that operator 

    std::size_t const NUM_BYTES_C = sizeof( double ) * numBlocks;

  // Allocate device buffers and transfer input data to device.
        double clkbegin, clkend;
        double t;
        clkbegin = rtclock();
  cl::Buffer B(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
      b.size() * sizeof(double), b.data()); // input vector
    //    queue.finish();
        clkend = rtclock();
        t = clkend-clkbegin;
        std::cout << "time for data transfer of b = " << t*1.0e+9 << "ns" << std::endl;

        clkbegin = rtclock();
  cl::Buffer C(context, CL_MEM_READ_WRITE,
      c.size() * sizeof(double) ); // output vector
     //   queue.finish();
        clkend = rtclock();
        t = clkend-clkbegin;
        std::cout << "time for data transfer of c = " << t*1.0e+9 << "ns" << std::endl;

  // Set kernel parameters.
  reduce.setArg(0, static_cast<cl_ulong>(N));
  reduce.setArg(1, B);
  reduce.setArg(2, C);
  reduce.setArg(3, cl::Local( sizeof(double)*blockSize ) ); // declaring local memory externally

  
    double average_time = 0.0;
    double average_queue_time = 0.0;
    double average_submission_time = 0.0;
    double num_of_turns = 0.0;
    double average_time_cpu = 0.0;

    std::size_t const NUM_REPETITIONS = 20;//1010;

    for( std::size_t ll = 0 ; ll < NUM_REPETITIONS ; ++ll )
    {
        std::fill( c.begin(), c.end(), 0.0 ); // this is also not required, since 'c' is being over written in each iteration
        // 'b' is not being modified, so no need to pass it each time

        cl::Event event;

        cl_int ret = CL_SUCCESS;

        ret  = queue.enqueueWriteBuffer( C, CL_TRUE, 0, NUM_BYTES_C, c.data() ); // enquing the write buffer means sending the data to the device
        
        double clkbegin, clkend;
        double t;
        clkbegin = rtclock();
        ret |= queue.enqueueNDRangeKernel( 
            reduce, cl::NullRange, cl::NDRange( N_sz ), 
            cl::NDRange( blockSize ), nullptr, &event );

        queue.flush();

        assert( ret == CL_SUCCESS );
        event.wait();
        clkend = rtclock();

        t = clkend-clkbegin;

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
      // enquing the read buffer means sending the data to the host
        /* enqueueReadBuffer() is an implicit queue.flush() */

        assert( ret == CL_SUCCESS );

        double const kernel_time_elapsed = when_kernel_ended - when_kernel_started;
        double const kernel_queue_time_elapsed = when_kernel_submitted - when_kernel_queued;
        double const kernel_submission_time_elapsed = when_kernel_started - when_kernel_submitted;

        if( ll < 20u )
        {
            num_of_turns += 1.0;
            average_time += (kernel_time_elapsed-average_time)/num_of_turns; // finding the running average
            average_queue_time = (kernel_queue_time_elapsed - average_queue_time)/num_of_turns;
            average_submission_time = (kernel_submission_time_elapsed - average_submission_time)/num_of_turns;
            average_time_cpu += (t-average_time_cpu)/num_of_turns; // finding the average cpu running
          std::cout << "kernel_time_elapsed in iteration " << ll << " : " << kernel_time_elapsed << std::endl;
          std::cout << "kernel_queue_time_elapsed in iteration " << ll << " : " << kernel_queue_time_elapsed << std::endl;
          std::cout << "kernel_submission_time_elapsed in iteration " << ll << " : " << kernel_submission_time_elapsed << std::endl;
          std::cout << "time as measured on cpu in iteration " << ll << " : " << t*1.0e+9 << std::endl;
          std::cout << "--------------------\n";
        }

        double sum_reduce = 0.0;

        for( auto const& partial_sum : c ) // finding the block-wise sum on the host
        {
            sum_reduce += partial_sum;
        }

//        std::cout << "ll = " << std::setw( 5 ) << ll 
//                  << " :: sum_reduce = " << sum_reduce << std::endl;
      if(ll == NUM_REPETITIONS -1)
        std::cout << "sum = \n" << sum_reduce << "\n";
    }
    
//    if( num_of_turns > 1.0 )
//    {
//        average_time /= num_of_turns;
//    }
    
    std::cout << "average time taken = " << average_time << "ns" << std::endl;
    std::cout << "average queue time taken = " << average_queue_time << "ns" << std::endl;
    std::cout << "average submission time taken = " << average_submission_time << "ns" << std::endl;
    std::cout << "average time taken as measured on cpu = " << average_time_cpu*1.0e+9 << "ns" << std::endl;
    
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
      std::cout << jj<<":"<<devices[jj].getInfo<CL_DEVICE_NAME>() << std::endl;
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
