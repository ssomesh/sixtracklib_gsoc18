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
create a *build* directory and configure the build process by providing the path to the *external* subdirectory in *sixtrack-v0*:
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

Inside ${SIXTRACK_DIR}, enter the build directory and run cmake and make:
```
cd ${SIXTRACK_DIR}/build
cmake ..
make
```
## Executing the executables

**Note**: 
* The name of the executable for a source code has the same name as that of the **.cpp** file.
* Example: For the source file *parallel_beam_elements_allinsequence.cpp*, the executable created in the *build* folder upon a successful build is named *parallel_beam_elements_allinsequence*  


In order to replicate the performance numbers reported for the various scenarios in the folder [*benchmarking_results*](benchmarking_results), the executables of the source codes of interest need to be executed as follows:

```
./parallel_beam_elements-finalversion <#particles> <#turns>  [deviceIdx]
```
    
```
./parallel_beam_elements_allinsequence <#particles> <#turns> [deviceIdx]
```
```
./parallel_beam_elements_oneatatime <#particles> <#turns> <tracking_function_id> [deviceIdx]
```
```
./parallel_beam_elements_switchcaseoncpu <#particles> <#turns>  [deviceIdx]
```
```
./parallel_beam_elements_switchcaseoncpuremoved <#particles> <#turns>  [deviceIdx]
```
