// This code finds the sum of the elements of a vector 
//NOTE: The reduce kernel expects the blockSize to be a "power of 2" to work correctly. Modify the code so that it works for any blockSize
// Profiling the code 

#include <iostream>
#include <vector>
#include <string>
#include <assert.h>

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
//"    if(gid >= n)\n"
//"{\n"
//"       return;\n"
//"}\n"
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

int nextPowerOf2(int n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
} 

int mk_test(std::vector<cl::Device> devices, int ndev,  cl::Context context) {
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
  size_t N = 3225; 
  std::cout << "N = " << N << std::endl;
  // Prepare input data.
  const size_t blockSize = 512; // # of threads per work-group (set by default is 256)
  assert((blockSize & (blockSize-1)) == 0); // ensure blockSize is a power of 2 
  size_t numBlocks = (N+blockSize-1)/blockSize; // the ceil of N/blockSize
  size_t N_sz = numBlocks * blockSize; // The multiple of blockSize greater than N and closest to N
  //std::cout << "N_sz = " << N_sz << std::endl;
  std::vector<double> b(N_sz, 0); 

  std::vector<double> c(numBlocks);

  for (int jj=0; jj<N; jj++){
    b[jj]=2.0*jj;
  };
 // The index [N,N_sz-1] are padded with zeros; in general it can be padded with the identity element for that operator 

  // Allocate device buffers and transfer input data to device.
  cl::Buffer B(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
      b.size() * sizeof(double), b.data()); // input vector

  cl::Buffer C(context, CL_MEM_READ_WRITE,
      c.size() * sizeof(double)); // output vector

  // Set kernel parameters.
  reduce.setArg(0, static_cast<cl_ulong>(N));
  reduce.setArg(1, B);
  reduce.setArg(2, C);

  //print B
//  std::cout << "Print B\n";
//  for(int i=0; i<N_sz;++i)
//  {
//    std::cout << b[i] ;
//    std::cout << (((i%blockSize) != (blockSize-1)) ? "\t" : "\n"); // printing 8 numbers on a line
//  }
//  std::cout << std::endl;

  cl::Event event;
  // Launch kernel on the compute device.
  cl_int success =  queue.enqueueNDRangeKernel(
  //queue.enqueueTask( 
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

  double elapsed = when_started-when_ended;
  std::cout << "kernel ran for " << elapsed << " ns" << std::endl;

  // Get result back to host; block until complete
  queue.enqueueReadBuffer(C, CL_TRUE, 0, c.size() * sizeof(double), c.data());



//  std::cout << "Print C\n";
//  // print C
//  for(int i=0; i<numBlocks;++i) // i < #of work-groups; each entry corresponds to a work-group
//    std::cout << c[i] << "\n";

// NOTE: c contains the block-wise sum of the input vector
// out[0] contains the final sum
// The size of the problem and the number of threads per work-group have been carefully set to make sure that out[0] contains the final answer.
// i.e. the size of the intermediate array, c, is equal to the size of the work-group with which the next launch to the 'reduce' kernel is made.

#if 1
  const int blockSizeNew = nextPowerOf2(numBlocks);//the nearest power of 2 greater than numBlocks
 // std::cout << "blockSizeNew = " << blockSizeNew << std::endl;

  std::vector<double> out(1); // a vector of size 1, since it will contain only the final answer

  std::vector<double> tmp(blockSizeNew,0);
  for(int i=0; i<numBlocks;++i) 
    tmp[i] =  c[i];
  
     

  // Allocate device buffers and transfer input data to device.
  cl::Buffer TMP(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
      tmp.size() * sizeof(double), tmp.data()); // input vector

  cl::Buffer OUT(context, CL_MEM_READ_WRITE,
      out.size() * sizeof(double)); // output vector

  // Set kernel parameters.
  reduce.setArg(0, static_cast<cl_ulong>(N));
  reduce.setArg(1, TMP);
  reduce.setArg(2, OUT);
  

  // Launch kernel on the compute device.
  // @assert( number of blocks is 1; i.e. the blockSize for this kernel launch is >= numBlocks since we want the barrier to be there and get the final answer
  queue.enqueueNDRangeKernel(
      reduce, 
      cl::NullRange, // an offset to compute the global id 
      cl::NDRange(blockSizeNew), // the number of work-items (threads) spawned along each direction; can be 1D,2D,3D.. i.e. NDRange(x,y,z); 
      cl::NDRange(blockSizeNew) // the number of work items (threads) per group
      //cl::NullRange// If you pass NULL (or cl::NullRange) to the last parameter (the # of threads per block), the OpenCL implementation will try to break down the threads into an optimal (for some optimisation strategy) value.      
      );

  // Get result back to host; block until complete
  queue.enqueueReadBuffer(OUT, CL_TRUE, 0, out.size() * sizeof(double), out.data());
  
  std::cout <<  "Sum = " << out[0] << std::endl;
#endif
};

int main(int argc, char *argv[]) {

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
      mk_test(devices,jj,context);
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
