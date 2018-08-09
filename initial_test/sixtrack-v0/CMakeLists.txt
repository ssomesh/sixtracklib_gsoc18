cmake_minimum_required( VERSION 3.3 )

project( sixtracklib_gsoc2018_v0 )

# We know where to find the SixTracklib installation -> 
# set the SixTracklib_DIR variable to help CMake finding it:

set( SixTracklib_DIR "${CMAKE_SOURCE_DIR}/external/lib/cmake" )
find_package( SixTracklib REQUIRED )

find_package( OpenCL REQUIRED )
include_directories( ${OpenCL_INCLUDE_DIR} )

#-------------------------------------------------------------------
# parallel_drift

add_executable( parallel_drift parallel_drift.cpp )
             
target_link_libraries( parallel_drift SixTracklib::sixtrack ${OpenCL_LIBRARY} )

set_property( TARGET parallel_drift PROPERTY CXX_STANDARD 11 )
set_property( TARGET parallel_drift PROPERTY CXX_STANDARD_REQUIRED ON )
set_property( TARGET parallel_drift PROPERTY COMPILE_OPTIONS -Wall -Werror -pedantic) 

#-------------------------------------------------------------------
# parallel_drift_cleanedup

add_executable( parallel_drift_cleanedup parallel_drift_cleanedup.cpp )
             
target_link_libraries( parallel_drift_cleanedup SixTracklib::sixtrack ${OpenCL_LIBRARY} )

set_property( TARGET parallel_drift_cleanedup PROPERTY CXX_STANDARD 11 )
set_property( TARGET parallel_drift_cleanedup PROPERTY CXX_STANDARD_REQUIRED ON )
set_property( TARGET parallel_drift_cleanedup PROPERTY COMPILE_OPTIONS -Wall -Werror -pedantic) 

#-------------------------------------------------------------------
# parallel_beam_elements

add_executable( parallel_beam_elements parallel_beam_elements.cpp )
             
target_link_libraries( parallel_beam_elements SixTracklib::sixtrack ${OpenCL_LIBRARY} )

set_property( TARGET parallel_beam_elements PROPERTY CXX_STANDARD 11 )
set_property( TARGET parallel_beam_elements PROPERTY CXX_STANDARD_REQUIRED ON )
set_property( TARGET parallel_beam_elements PROPERTY COMPILE_OPTIONS -Wall -Werror -pedantic) 


#-------------------------------------------------------------------
# parallel_beam_elements_opt1

add_executable( parallel_beam_elements_opt1 parallel_beam_elements_opt1.cpp )
             
target_link_libraries( parallel_beam_elements_opt1 SixTracklib::sixtrack ${OpenCL_LIBRARY} )

set_property( TARGET parallel_beam_elements_opt1 PROPERTY CXX_STANDARD 11 )
set_property( TARGET parallel_beam_elements_opt1 PROPERTY CXX_STANDARD_REQUIRED ON )
set_property( TARGET parallel_beam_elements_opt1 PROPERTY COMPILE_OPTIONS -Wall -Werror -pedantic -g) 

#-------------------------------------------------------------------
# parallel_beam_elements_opt2

add_executable( parallel_beam_elements_opt2 parallel_beam_elements_opt2.cpp )
             
target_link_libraries( parallel_beam_elements_opt2 SixTracklib::sixtrack ${OpenCL_LIBRARY} )

set_property( TARGET parallel_beam_elements_opt2 PROPERTY CXX_STANDARD 11 )
set_property( TARGET parallel_beam_elements_opt2 PROPERTY CXX_STANDARD_REQUIRED ON )
set_property( TARGET parallel_beam_elements_opt2 PROPERTY COMPILE_OPTIONS -Wall -Werror -pedantic -g) 

#-------------------------------------------------------------------

# parallel_beam_elements_opt3

add_executable( parallel_beam_elements_opt3 parallel_beam_elements_opt3.cpp )
             
target_link_libraries( parallel_beam_elements_opt3 SixTracklib::sixtrack ${OpenCL_LIBRARY} )

set_property( TARGET parallel_beam_elements_opt3 PROPERTY CXX_STANDARD 11 )
set_property( TARGET parallel_beam_elements_opt3 PROPERTY CXX_STANDARD_REQUIRED ON )
set_property( TARGET parallel_beam_elements_opt3 PROPERTY COMPILE_OPTIONS -Wall -Werror -pedantic -g) 

#-------------------------------------------------------------------
#sequential drift_cpu

add_executable( drift_cpu drift_cpu.cpp )
target_link_libraries( drift_cpu SixTracklib::sixtrack )

set_property( TARGET drift_cpu PROPERTY CXX_STANDARD 11 )
set_property( TARGET drift_cpu PROPERTY CXX_STANDARD_REQUIRED ON )
set_property( TARGET drift_cpu PROPERTY COMPILE_OPTIONS -Wall -Werror -pedantic )
#-------------------------------------------------------------------

# parallel_beam_elements-finalversion

add_executable( parallel_beam_elements-finalversion parallel_beam_elements-finalversion.cpp )
             
target_link_libraries( parallel_beam_elements-finalversion SixTracklib::sixtrack ${OpenCL_LIBRARY} )

set_property( TARGET parallel_beam_elements-finalversion PROPERTY CXX_STANDARD 11 )
set_property( TARGET parallel_beam_elements-finalversion PROPERTY CXX_STANDARD_REQUIRED ON )
set_property( TARGET parallel_beam_elements-finalversion PROPERTY COMPILE_OPTIONS -Wall -Werror -pedantic) 


#-------------------------------------------------------------------
# parallel_beam_elements_oneatatime

add_executable( parallel_beam_elements_oneatatime parallel_beam_elements_oneatatime.cpp )
             
target_link_libraries( parallel_beam_elements_oneatatime SixTracklib::sixtrack ${OpenCL_LIBRARY} )

set_property( TARGET parallel_beam_elements_oneatatime PROPERTY CXX_STANDARD 11 )
set_property( TARGET parallel_beam_elements_oneatatime PROPERTY CXX_STANDARD_REQUIRED ON )
set_property( TARGET parallel_beam_elements_oneatatime PROPERTY COMPILE_OPTIONS -Wall -Werror -pedantic) 


#-------------------------------------------------------------------
