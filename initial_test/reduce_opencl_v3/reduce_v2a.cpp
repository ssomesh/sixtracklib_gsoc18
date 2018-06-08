// This code finds the sum of the elements of a vector 
// local memory is used -- declared externally
//FIXME: The reduce kernel expects the blockSize to be a "power of 2" to work correctly. Modify the code so that it works for any blockSize

#include <iostream>
#include <vector>
#include <string>
#include <assert.h>
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
  std::vector<double> c(numBlocks, 0.0);

  for (int jj=0; jj<N; jj++)
  {
    b[jj]=1.0;//2.0 * jj;
  };
 // The index [N,N_sz-1] are padded with zeros; in general it can be padded with the identity element for that operator 


        double clkbegin, clkend;
        double t;
        clkbegin = rtclock();
  cl::Buffer B(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
      b.size() * sizeof(double), b.data()); // input vector
      //  queue.finish();
        clkend = rtclock();
        t = clkend-clkbegin;
        std::cout << "time for data transfer of b = " << t*1.0e+9 << "ns" << std::endl;

        clkbegin = rtclock();
  cl::Buffer C(context, CL_MEM_READ_WRITE,
      c.size() * sizeof(double) ); // output vector
      //  queue.finish();
        clkend = rtclock();
        t = clkend-clkbegin;
        std::cout << "time for data transfer of c = " << t*1.0e+9 << "ns" << std::endl;


  // Set kernel parameters.
  reduce.setArg(0, static_cast<cl_ulong>(N));
  reduce.setArg(1, B);
  reduce.setArg(2, C);
  reduce.setArg(3, cl::Local( sizeof(double)*blockSize ) ); // declaring local memory externally

  //print B
//  std::cout << "Print B\n";
//  for(int i=0; i<N_sz;++i)
//  {
//    std::cout << b[i] ;
//    std::cout << (((i%blockSize) != (blockSize-1)) ? "\t" : "\n"); // printing 8 numbers on a line
//  }
//  std::cout << std::endl;

//  double average_time = 0.0f;
//  for(int i=0; i<1010;++i) {
  cl::Event event;
  // Launch kernel on the compute device.

//  double clkbegin, clkend;
//  double t;
  clkbegin = rtclock();

  cl_int success =  queue.enqueueNDRangeKernel(
      reduce, 
      cl::NullRange, // an offset to compute the global id 
      cl::NDRange(N_sz), // the number of work-items (threads) spawned along each direction; can be 1D,2D,3D.. i.e. NDRange(x,y,z); 
      // since we can't specify the number of work_groups directly, launch number of threads more than N -- N_sz elements to be precise
      cl::NDRange(blockSize), // the number of work items (threads) per group
     // cl::NullRange;// If you pass NULL (or cl::NullRange) to the last parameter (the # of threads per block), the OpenCL implementation will try to break down the threads into an optimal (for some optimisation strategy) value.      
      nullptr,
      &event
      );

  assert( success == CL_SUCCESS );
  queue.flush();
  event.wait();
//  queue.finish();
  clkend = rtclock();

  t = clkend-clkbegin;
  //queue.finish();
  cl_ulong when_queued = 0;
  cl_ulong when_submitted = 0;
  cl_ulong when_started = 0;
  cl_ulong when_ended = 0;

  success = event.getProfilingInfo< cl_ulong >( CL_PROFILING_COMMAND_QUEUED, &when_queued );
  assert( success == CL_SUCCESS );
  success = event.getProfilingInfo< cl_ulong >( CL_PROFILING_COMMAND_SUBMIT, &when_submitted );
  assert( success == CL_SUCCESS );
  success = event.getProfilingInfo< cl_ulong >( CL_PROFILING_COMMAND_START, &when_started );
  assert( success == CL_SUCCESS );
  success = event.getProfilingInfo< cl_ulong >( CL_PROFILING_COMMAND_END, &when_ended );
  assert( success == CL_SUCCESS );
  double elapsed = when_ended-when_started;
//  if(i > 10) // ignoring the first 10 measurements of time
//    average_time += elapsed/1000.0;

//  }
//  std::cout << "kernel ran for " << average_time << " ns" << std::endl;
  std::cout << "kernel ran for " << elapsed << " ns" << std::endl;
  std::cout << "kernel queuing time = " << when_submitted - when_queued << "ns" << std::endl;
  std::cout << "kernel submission time = " << when_started - when_submitted << "ns" << std::endl;
    std::cout << "time taken as measured on cpu = " << t*1.0e+9 << "ns" << std::endl;
  // Get result back to host; block until complete
  
  queue.enqueueReadBuffer(C, CL_TRUE, 0, c.size() * sizeof(double), c.data());
  queue.flush();

  double sum_reduce = 0.0;
  
  for( auto const& partial_sum : c ) // finding the block-wise sum on the host
  {
      sum_reduce += partial_sum;
  }
  
  std::cout << "Print sum_reduce: \n" << sum_reduce << std::endl;
  
//  // print C
//  for(int i=0; i<numBlocks;++i) // i < #of work-groups; each entry corresponds to a work-group
//    std::cout << c[i] << "\n";

// NOTE: c contains the block-wise sum of the input vector
// sum_reduce contains the final sum

};

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
