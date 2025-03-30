cmake_minimum_required(VERSION 3.10)

project(dOpenCLstandalonetest)

# The OpenCL C++ bindings actually require linking with the thread library:
# https://github.com/KhronosGroup/OpenCL-CLHPP/blob/0192662af0156acc6346e0d398eed6e2bb4274f3/include/CL/cl2.hpp#L2124
# Otherwise, the build can succeed, but if the threads library has not been
# linked somehow, it will fail at runtime:  https://stackoverflow.com/q/51584960
set(THREADS_PREFER_PTHREAD_FLAG TRUE) # Required for Ubuntu 16.04 - CMake 3.5.1 - PPC64LE
find_package(Threads REQUIRED)

find_package(TIFF)
include_directories(${TIFF_INCLUDE_DIRS})
link_directories(${TIFF_LIBRARY_DIRS})

foreach(test bitonic
             mapreduce_addition
             test_createbuffer_ptr
             test_createbuffer_ptr_multi
             test_createbuffer_ptr_race
             test_explicit_copy_cl
             test_implicit_copy_cl
             read_write_compressed
             text_search)
	add_executable(${test} src/${test}.cpp)
	target_link_libraries(${test} OpenCL Threads::Threads)
	target_compile_definitions(${test} PRIVATE CL_TARGET_OPENCL_VERSION=120
	                                           CL_HPP_MINIMUM_OPENCL_VERSION=120
	                                           CL_HPP_TARGET_OPENCL_VERSION=120
	                                           CL_HPP_ENABLE_EXCEPTIONS)
endforeach(test)

if (TIFF_FOUND)
	add_executable(downsize_tiff src/downsize_tiff.cpp)
	target_link_libraries(downsize_tiff OpenCL ${TIFF_LIBRARIES} Threads::Threads)
	target_compile_definitions(downsize_tiff PRIVATE CL_TARGET_OPENCL_VERSION=120
	                                                 CL_HPP_MINIMUM_OPENCL_VERSION=120
	                                                 CL_HPP_TARGET_OPENCL_VERSION=120
	                                                 CL_HPP_ENABLE_EXCEPTIONS)
endif()
