# Installation

## Prerequisities
* cmake 3.8 or higher
* gcc and g++ 5.4.x or higher (C++11 support)
* The dependencies for building the submodules of sixtracklib

## Preparation: Installing sixtracklib into the external subfolder of *sixtrack-v0*
**Assumptions**:
* this document is located under ${SIXTRACK_DIR} 
* Upstream sixtracklib will be cloned into ${HOME}/git/sixtracklib

```
cd ${HOME}/git
git clone git@github.com:martinschwinzerl/sixtracklib.git 
cd sixtracklib.git
```
create a build directory and configure the build process by providing the path to the *external* subdirectory in *sixtrack-v0*:
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=${SIXTRACK_DIR}/external -DGTEST_ROOT=/opt/googletest
```
The output should contain no errors. Then build and install
```
make
make install
cd ${SIXTRACK_DIR}
```
Verify that the contents of external look something like this:
```
external/
├── include
│   └── sixtracklib
│       ├── common
│       │   ├── alignment.h
│       │   ├── beam_elements.h
│       │   ├── blocks.h
│       │   ├── impl
│       │   │   ├── alignment_impl.h
│       │   │   ├── beam_elements_api.h
│       │   │   ├── beam_elements_type.h
│       │   │   ├── particles_api.h
│       │   │   └── particles_type.h
│       │   ├── mem_pool.h
│       │   └── particles.h
│       └── _impl
│           ├── definitions.h
│           ├── modules.h
│           ├── namespace_begin.h
│           ├── namespace_end.h
│           └── path.h
└── lib
    ├── cmake
    │   ├── SixTracklibConfig.cmake
    │   ├── SixTracklibTargets.cmake
    │   └── SixTracklibTargets-debug.cmake
    └── libsixtrackd.so
```
## Building the executables
Inside ${STUDY8_DIR}, enter the build directory and run cmake and make:
```
cd ${SIXTRACK_DIR}/build
cmake ..
make
```




<!--
# Study 8 - Use external sixtracklib installation for a minimal program
This is one quick & dirty way to use the upstream sixtracklib repository for a simple demo program or for checking/comparison, purposes. To this end, a recent upstream version of sixtracklib is installed into the "external" folder in this study and CMake is used to configure the demo program - use_blocks.cpp - to properly use it.

**Note**: The proper way to do this would be to use git submodules; this is not the proper way to do this, but probably easier to understand

## Prerequisities
* cmake 3.3 or higher
* gcc and g++ 5.4.x or higher (C++11 support)
* The dependencies for building the submodules of sixtracklib; here we aim at a minimal build with no external dependencies so if you stick to the instructions below, there are no further requirements 

## Preparation: Installing sixtracklib into the external subfolder of study8
**Assumptions**:
* this document is located under ${STUDY8_DIR} 
* Upstream sixtracklib will be cloned into ${HOME}/git/sixtracklib

```
cd ${HOME}/git
git clone git@github.com:martinschwinzerl/sixtracklib.git 
cd sixtracklib.git
```
We will now verify that all optional backens are disabled to allow for a minimal build:
```
tail --lines=10 Settings.cmake.default
```
The output should be something like this => note that all backends should be disabled (i.e. OFF rather than ON); otherwise, please switch them to OFF
```
option( SIXTRACKL_ENABLE_AUTOVECTORIZATION "Enable Autovectorization"       OFF )
option( SIXTRACKL_ENABLE_MANUAL_SIMD       "Enable manual SIMD backend"     OFF )
option( SIXTRACKL_ENABLE_OPENMP            "Enable OpenMP backend"          OFF )
option( SIXTRACKL_ENABLE_OPENCL            "Enable OpenCL backend"          OFF )
option( SIXTRACKL_ENABLE_CUDA              "Enable CUDA backend"            OFF )
```
Then create a build directory and configure the build process by providing the path to the external subdirectory in study 8:
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=${STUDY8_DIR}/external -DGTEST_ROOT=/opt/googletest
```
The output should contain no errors. Then build and install by the usual combination of 
```
make
make install
cd ${STUDY8_DIR}
```
Verify that the contents of external look something like this:
```
external/
├── include
│   └── sixtracklib
│       ├── common
│       │   ├── alignment.h
│       │   ├── beam_elements.h
│       │   ├── blocks.h
│       │   ├── impl
│       │   │   ├── alignment_impl.h
│       │   │   ├── beam_elements_api.h
│       │   │   ├── beam_elements_type.h
│       │   │   ├── particles_api.h
│       │   │   └── particles_type.h
│       │   ├── mem_pool.h
│       │   └── particles.h
│       └── _impl
│           ├── definitions.h
│           ├── modules.h
│           ├── namespace_begin.h
│           ├── namespace_end.h
│           └── path.h
└── lib
    ├── cmake
    │   ├── SixTracklibConfig.cmake
    │   ├── SixTracklibTargets.cmake
    │   └── SixTracklibTargets-debug.cmake
    └── libsixtrackd.so
```
## Building the Study 8 example program
Inside ${STUDY8_DIR}, enter the build directory and run cmake and make:
```
cd ${STUDY8_DIR}/build
cmake ..
make
```
You should be able to run use_blocks and get an ouput similiar to the one provided below:
```
cd ${STUDY8_DIR}/build
./use_blocks

Print these newly created beam_elements: 

     0 | type: drift        | length =        0.2 [m] 
     1 | type: drift        | length =        0.2 [m] 
     2 | type: drift        | length =        0.2 [m] 
     3 | type: drift        | length =        0.2 [m] 
     4 | type: drift        | length =        0.2 [m] 
     5 | type: drift        | length =        0.2 [m] 
     6 | type: drift        | length =        0.2 [m] 
     7 | type: drift        | length =        0.2 [m] 
     8 | type: drift        | length =        0.2 [m] 
     9 | type: drift_exact  | length =        0.1 [m] 

Print the copied beam_elements: 

     0 | type: drift        | length =        0.2 [m] 
     1 | type: drift        | length =        0.2 [m] 
     2 | type: drift        | length =        0.2 [m] 
     3 | type: drift        | length =        0.2 [m] 
     4 | type: drift        | length =        0.2 [m] 
     5 | type: drift        | length =        0.2 [m] 
     6 | type: drift        | length =        0.2 [m] 
     7 | type: drift        | length =        0.2 [m] 
     8 | type: drift        | length =        0.2 [m] 
     9 | type: drift_exact  | length =        0.1 [m] 


Finished successfully!
```
-->

