cmake_minimum_required(VERSION 3.10)

project(dOpenCLicd)

option(USE_MEM_LOCK "Use page-locked host memory (experimental, Linux only)" OFF)

set(DOPENCL_INCLUDE_DIR "${dOpenCLlib_SOURCE_DIR}/include" CACHE PATH "Path to dOpenCL headers")
set(DOPENCL_LIBRARY_DIR "${dOpenCLlib_BINARY_DIR}" CACHE PATH "Path to dOpenCL library")

# search for Boost String Algorithms Library
find_package(Boost 1.32.0 REQUIRED)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src *.cpp)
#aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCES)

include_directories(${DOPENCL_INCLUDE_DIR}
                    ${Boost_INCLUDE_DIRS})

link_directories(${DOPENCL_LIBRARY_DIR}
                 ${Boost_LIBRARY_DIRS})

# 
# dOpenCL ICD target
#

add_library (dOpenCL SHARED ${SOURCES})
# TODO Library type must be 'MODULE', such that it can be plugged into the ICD loader using dlopen-like functionality
#add_library (dOpenCL MODULE ${SOURCES})
target_link_libraries (dOpenCL
	dcl
	${Boost_LIBRARIES})

target_compile_definitions(dOpenCL PRIVATE CL_TARGET_OPENCL_VERSION=120
                                           CL_HPP_MINIMUM_OPENCL_VERSION=120
                                           CL_HPP_TARGET_OPENCL_VERSION=120
                                           CL_HPP_ENABLE_EXCEPTIONS)

# enable OpenCL 1.1/1.2 compatibility
target_compile_definitions(dOpenCL PRIVATE CL_USE_DEPRECATED_OPENCL_1_1_APIS
                                           CL_USE_DEPRECATED_OPENCL_1_2_APIS)
if(USE_MEM_LOCK)
	target_compile_definitions(dOpenCL PRIVATE DCL_MEM_LOCK)
endif(USE_MEM_LOCK)
# bind references to global symbols to the definition within the shared library (obsolete);
# set library version and SONAME
set_target_properties(dOpenCL PROPERTIES
#	LINK_FLAGS -Bsymbolic-functions
	VERSION 0.4.0
	SOVERSION 0)

if(UNIX)
# TODO Create vendor file for ICD loader (UNIX only)
	# File will be placed in /etc/OpenCL/vendors during installation
	#file(WRITE wwu.icd "$<TARGET_FILE:dOpenCL>")
endif(UNIX)
if(WIN32)
#TODO Create registry entry for ICD loader (Windows only)
endif(WIN32)


#
# install configuration
#

install(TARGETS dOpenCL
	DESTINATION lib)
