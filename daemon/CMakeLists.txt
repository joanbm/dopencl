cmake_minimum_required(VERSION 3.10)

project(dOpenCLdaemon)

option(FORCE_FLUSH "Force flush after enqueued command (recommended for AMD OpenCL runtime)" OFF)

set(DOPENCL_INCLUDE_DIR "${dOpenCLlib_SOURCE_DIR}/include" CACHE PATH "Path to dOpenCL headers")
set(DOPENCL_LIBRARY_DIR "${dOpenCLlib_BINARY_DIR}" CACHE PATH "Path to dOpenCL library")

# search for Boost.Program_options
find_package(Boost 1.41.0 COMPONENTS program_options REQUIRED)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src *.cpp)
#aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCES)

include_directories(${DOPENCL_INCLUDE_DIR}
                    ${Boost_INCLUDE_DIRS})

link_directories(${DOPENCL_LIBRARY_DIR}
                 ${Boost_LIBRARY_DIRS})


#
# dOpenCL daemon target
#

add_executable (dcld ${SOURCES})
target_link_libraries(dcld
	dcl
	OpenCL
	${Boost_LIBRARIES})
target_compile_definitions(dcld PRIVATE CL_TARGET_OPENCL_VERSION=120
                                        CL_HPP_MINIMUM_OPENCL_VERSION=120
                                        CL_HPP_TARGET_OPENCL_VERSION=120
                                        CL_HPP_ENABLE_EXCEPTIONS)

# enable OpenCL 1.1/1.2 compatibility
target_compile_definitions(dcld PRIVATE CL_USE_DEPRECATED_OPENCL_1_1_APIS
                                        CL_USE_DEPRECATED_OPENCL_1_2_APIS)
if (FORCE_FLUSH)
	target_compile_definitions(dcld PRIVATE FORCE_FLUSH)
endif (FORCE_FLUSH)


#
# install configuration
#

install(TARGETS dcld
	DESTINATION bin)

