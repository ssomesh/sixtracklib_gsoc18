
#include <stdio.h>
#include <CL/cl.hpp>
#include <string>

int main(int argc, char ** argv) {
  static const char skernel[] = 
    "__kernel void\n" \
    "vectorAdd(__global const float * a,\n" \
    "__global const float * b,\n" \
    "__global\n" \
    "float * c)\n" \
    "{\n" \
    "// Vector element index\n" \
    "int nIndex = get_global_id(0);\n" \
    "c[nIndex] = a[nIndex] + b[nIndex];\n" \
    "}\n";

    // Kernel launch configuration
    const unsigned int cnBlockSize = 512;
  const unsigned int cnBlocks = 3;
  const unsigned int cnDimension = cnBlocks * cnBlockSize;

  // Get OpenCL platform count
  cl_uint NumPlatforms;
  clGetPlatformIDs (0, NULL, &NumPlatforms);

  // Get all OpenCL platform IDs
  cl_platform_id* PlatformIDs;
  PlatformIDs = new cl_platform_id[NumPlatforms];
  clGetPlatformIDs(NumPlatforms, PlatformIDs, NULL); 

  // Select NVIDIA platform (this example assumes it IS present)
  char cBuffer[1024];
  cl_uint NvPlatform;
  for(cl_uint i = 0; i < NumPlatforms; ++i)
  {
    clGetPlatformInfo (PlatformIDs[i], CL_PLATFORM_NAME, 1024, cBuffer, NULL);
    if(strstr(cBuffer, "NVIDIA") != NULL)
    {
      NvPlatform = i;
      break;
    }
  }
  cl_device_id cdDevice;
  clGetDeviceIDs(PlatformIDs[NvPlatform], CL_DEVICE_TYPE_GPU, 1, &cdDevice, NULL);
  // Create a context
  cl_context hContext;
  hContext = clCreateContext(0, 1, &cdDevice, NULL, NULL, NULL);

  // Create a command queue for the device in the context
  cl_command_queue hCmdQueue;
  hCmdQueue = clCreateCommandQueue(hContext, cdDevice, 0, NULL);

  // Create & compile program
  cl_program hProgram;
  hProgram = clCreateProgramWithSource(hContext, 1, skernel, 0, 0);
  clBuildProgram(hProgram, 0, 0, 0, 0, 0);

  // Create kernel instance
  cl_kernel hKernel;
  hKernel = clCreateKernel(hProgram, "vectorAdd", 0);

  // Allocate host vectors
  float * pA = new float[cnDimension];
  float * pB = new float[cnDimension];
  float * pC = new float[cnDimension];

  // intialize the host vectors 
  for(int i=0; i<cnDimension; ++i) {
    pA[i] = i+1.0f;
    pB[i] = 2.0f*(i+1);
  }

  // Allocate device memory (and init hDeviceMemA and hDeviceMemB)
  cl_mem hDeviceMemA, hDeviceMemB, hDeviceMemC;
  hDeviceMemA = clCreateBuffer(hContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,cnDimension * sizeof(cl_float), pA, 0);
  hDeviceMemB = clCreateBuffer(hContext,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cnDimension * sizeof(cl_float), pB, 0);
  hDeviceMemC = clCreateBuffer(hContext, CL_MEM_WRITE_ONLY, cnDimension * sizeof(cl_float), 0, 0);

  // Setup parameter values
  clSetKernelArg(hKernel, 0, sizeof(cl_mem), (void *)&hDeviceMemA);
  clSetKernelArg(hKernel, 1, sizeof(cl_mem), (void *)&hDeviceMemB);
  clSetKernelArg(hKernel, 2, sizeof(cl_mem), (void *)&hDeviceMemC);
  // Launch kernel
  clEnqueueNDRangeKernel(hCmdQueue, hKernel, 1, 0, &cnDimension, 0, 0, 0, 0);
  // Copy results from device back to host; block until complete
  clEnqueueReadBuffer(hContext, hDeviceMemC, CL_TRUE, 0, cnDimension * sizeof(cl_float), pC, 0, 0, 0);
  // Cleanup
  delete[] pA;
  delete[] pB;
  delete[] pC;
  delete[] PlatformIDs;
  clReleaseKernel(hKernel);
  clReleaseProgram(hProgram);
  clReleaseMemObj(hDeviceMemA);
  clReleaseMemObj(hDeviceMemB);
  clReleaseMemObj(hDeviceMemC);
  clReleaseCommandQueue(hCmdQueue);
  clReleaseContext(hContext);


  return 0;
}
