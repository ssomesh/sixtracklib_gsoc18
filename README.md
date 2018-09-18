## GSoC 2018: Optimize and Integrate Standalone Tracking Library (SixTrackLib)

As part of Google Summer of Code 2018, I developed a standalone minimal parallel implemetation of SixTrackLib.

## Documentation

A detailed documentation on the project can be found in [*GSoc2018Report.pdf*](GSoc2018Report.pdf).

## Prerequisities
* cmake 3.8 or higher
* gcc and g++ 5.4.x or higher (C++11 support)
* The dependencies for building the submodules of sixtracklib

## Getting Started

### Getting the code

- Clone the code into a directory called sixtracklib_gsoc18
    ```
    git clone --branch=master https://github.com/ssomesh/sixtracklib_gsoc18
    ```
- The relevant source codes for the parallel implementations of SixTrackLib is present in the folder [*initial_test/sixtrack-v0*](initial_test/sixtrack-v0)

### Installation

Please follow the instructions in the [Readme.md](initial_test/sixtrack-v0/Readme.md) in the folder [*initial_test/sixtrack-v0*](initial_test/sixtrack-v0).

## Source files of interest

The different minimal parallel implementations of SixTrackLib written during GSoC 2018 are the following (in [*initial_test/sixtrack-v0*](initial_test/sixtrack-v0)) :

- [parallel_beam_elements-finalversion.cpp](initial_test/sixtrack-v0/parallel_beam_elements-finalversion.cpp) and [kernels_beam_elements-finalversion.cl](initial_test/sixtrack-v0/kernels_beam_elements-finalversion.cl)
- [parallel_beam_elements_allinsequence.cpp](initial_test/sixtrack-v0/parallel_beam_elements_allinsequence.cpp) and [kernels_beam_elements_oneatatime.cl](initial_test/sixtrack-v0/kernels_beam_elements_oneatatime.cl)
- [parallel_beam_elements_oneatatime.cpp](initial_test/sixtrack-v0/parallel_beam_elements_oneatatime.cpp) and [kernels_beam_elements_oneatatime.cl](initial_test/sixtrack-v0/kernels_beam_elements_oneatatime.cl)
- [parallel_beam_elements_switchcaseoncpu.cpp](initial_test/sixtrack-v0/parallel_beam_elements_switchcaseoncpu.cpp) and [kernels_beam_elements_switchcaseoncpu.cl](initial_test/sixtrack-v0/kernels_beam_elements_switchcaseoncpu.cl)
- [parallel_beam_elements_switchcaseoncpuremoved.cpp](initial_test/sixtrack-v0/parallel_beam_elements_switchcaseoncpuremoved.cpp) and [kernels_beam_elements_switchcaseoncpuremoved.cl](initial_test/sixtrack-v0/kernels_beam_elements_switchcaseoncpuremoved.cl)

Note: In the above bulleted list, **\[\*\].cpp** contains the host code; **\[\*\].cl** contains the OpenCL device code

* The source files listed above have also been merged into the *master* branch of the **SixTrackLib** project. 
The source files can be found in **SixTrackLib** in the folder: 
https://github.com/martinschwinzerl/sixtracklib/tree/master/tests/benchmark/sixtracklib/opencl (starting with commit 93da10623)

## Generating the Results

The source files mentioned above generate benchmarking results for the various scenarios outlined in [*GSoc2018Report.pdf*](GSoc2018Report.pdf). 

The correpondence between a source file and a scenario is mentioned below.

- [parallel_beam_elements-finalversion.cpp](initial_test/sixtrack-v0/parallel_beam_elements-finalversion.cpp) : *Scenario 1(a)*
- [parallel_beam_elements_allinsequence.cpp](initial_test/sixtrack-v0/parallel_beam_elements_allinsequence.cpp) : *Scenario 1(b)*
- [parallel_beam_elements_oneatatime.cpp](initial_test/sixtrack-v0/parallel_beam_elements_oneatatime.cpp) : *Scenario 1(c)*
- [parallel_beam_elements_switchcaseoncpu.cpp](initial_test/sixtrack-v0/parallel_beam_elements_switchcaseoncpu.cpp) : *Scenario 2*
- [parallel_beam_elements_switchcaseoncpuremoved.cpp](initial_test/sixtrack-v0/parallel_beam_elements_switchcaseoncpuremoved.cpp) : *Scenario 3*

## Benchmarking Results

The benchmarking results obtained on running the various implementations (mentioned above) on various platforms (see [*GSoc2018Report.pdf*](GSoc2018Report.pdf)) are present in the folder [*initial_test/sixtrack-v0/benchmarking_results*](initial_test/sixtrack-v0/benchmarking_results).

- benchmarking_allinonekernel\[\*\] contains results for *Scenario 1(a)*
- benchmarking_allinsequence\[\*\] contains results for *Scenario 1(b)*
- benchmarking_oneatatime\[\*\] contains results for *Scenario 1(c)*
- benchmarking_switchcaseoncpu\[\*\] contains results for *Scenario 2*
- benchmarking_switchcaseoncpuremoved\[\*\] contains results for *Scenario 3*
