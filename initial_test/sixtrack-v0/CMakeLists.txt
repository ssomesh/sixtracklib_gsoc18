cmake_minimum_required( VERSION 3.3 )

project( sixtracklib_gsoc2018_study8 )

# We know where to find the SixTracklib installation -> 
# set the SixTracklib_DIR variable to help CMake finding it:

set( SixTracklib_DIR "${CMAKE_SOURCE_DIR}/external/lib/cmake" )
find_package( SixTracklib REQUIRED )

add_executable( parallel_drift parallel_drift.cpp )
             
target_link_libraries( parallel_drift SixTracklib::sixtrack )

set_property( TARGET parallel_drift PROPERTY CXX_STANDARD 11 )
set_property( TARGET parallel_drift PROPERTY CXX_STANDARD_REQUIRED ON )
set_property( TARGET parallel_drift PROPERTY COMPILE_OPTIONS -Wall -Werror -pedantic )

