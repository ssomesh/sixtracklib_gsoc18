// This code finds the work-group wise sum of the vector

#include <iostream>
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
"#  error double precision is not supported\n"
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


int mk_test(std::vector<cl::Device> devices, int ndev,  cl::Context context) {
  //std::cout << "Using " << devices[ndev].getInfo<CL_DEVICE_NAME>() << std::endl;

  // Create command queue.
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
    return 1;
  }

  cl::Kernel reduce(program, "reduce");
  //size_t N = 1 << 20;
  size_t N = 512;

  // Prepare input data.
  std::vector<double> b(N, 0);
  const size_t blockSize = 256; // # of threads per work-group (set by default)
  size_t numBlocks = (N+blockSize-1)/blockSize; // the ceil of N/blockSize

  std::vector<double> c(numBlocks);

  for (int jj=0; jj<N; jj++){
    b[jj]=2.0*jj;
  };


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
  std::cout << "Print B\n";
  for(int i=0; i<N;++i)
  {
    std::cout << b[i] ;
    std::cout << (((i%8) != 7) ? "\t" : "\n"); // printing 8 numbers on a line
  }
  std::cout << std::endl;

  // Launch kernel on the compute device.
  queue.enqueueNDRangeKernel(
      reduce, 
      cl::NullRange, 
      cl::NDRange(N),
      cl::NullRange
      );

  // Get result back to host; block until complete
  queue.enqueueReadBuffer(C, CL_TRUE, 0, c.size() * sizeof(double), c.data());



  std::cout << "Print C\n";
  // print C
  for(int i=0; i<numBlocks;++i) // i < #of work-groups; each entry corresponds to a work-group
    std::cout << c[i] << "\n";
// NOTE: C contains the block-wise sum of the input vector
  // TODO:Launch  the reduce kernel a second time to add the elements of the vector C

};

int main(int argc, char *argv[]) {
  //const size_t N = 1 << 20;
  const size_t N = 512; 

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
