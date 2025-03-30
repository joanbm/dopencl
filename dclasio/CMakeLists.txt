cmake_minimum_required(VERSION 3.10)

project(dOpenCLlib)

option(BUILD_SHARED_LIBS "Build shared library" ON)

# search for thread implementation (required for using C++'s std::thread)
find_package(Threads REQUIRED)

# search for Boost System Library (required by Boost.Asio) Library
find_package(Boost 1.35.0 COMPONENTS system REQUIRED)

# search for Doxygen
find_package(Doxygen)

# create lists of source files and headers
file(GLOB_RECURSE CL_WWU_HEADERS ${PROJECT_SOURCE_DIR}/include/CL *.h)
file(GLOB_RECURSE DCL_SOURCES ${PROJECT_SOURCE_DIR}/src/dcl *.cpp)
file(GLOB_RECURSE DCL_HEADERS ${PROJECT_SOURCE_DIR}/include/dcl *.h)
file(GLOB_RECURSE DCLASIO_SOURCES ${PROJECT_SOURCE_DIR}/src/dclasio *.cpp)

# include project headers
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${Boost_INCLUDE_DIRS})

link_directories(${Boost_LIBRARY_DIRS})


#
# dOpenCL library target
#

add_library(dcl ${DCL_SOURCES} ${DCLASIO_SOURCES})
target_link_libraries(dcl
	${Boost_LIBRARIES} Threads::Threads)

if (ENABLE_IO_LINK_COMPRESSION)
	target_link_libraries(dcl 842)
endif(ENABLE_IO_LINK_COMPRESSION)

target_compile_definitions(dcl PRIVATE CL_TARGET_OPENCL_VERSION=120
                                       CL_HPP_MINIMUM_OPENCL_VERSION=120
                                       CL_HPP_TARGET_OPENCL_VERSION=120
                                       CL_HPP_ENABLE_EXCEPTIONS)

# enable OpenCL 1.1/1.2 compatibility
target_compile_definitions(dcl PRIVATE CL_USE_DEPRECATED_OPENCL_1_1_APIS
                                       CL_USE_DEPRECATED_OPENCL_1_2_APIS)

if(BUILD_SHARED_LIBS)
# set library version and SONAME
	set_target_properties(dcl PROPERTIES
		VERSION 0.4.0
		SOVERSION 0)
else(BUILD_SHARED_LIBS)
# compile static library using -fPIC, such that it can be linked with a shared
# library, namely the dOpenCL ICD
	set_target_properties(dcl PROPERTIES
		COMPILE_FLAGS -fPIC)
endif(BUILD_SHARED_LIBS)

#
# Documentation target
#

# TODO Warn if Doxygen has not been found
# replace CMake variable in the Doxyfile.in template thereby generating Doxyfile
configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in
	${PROJECT_BINARY_DIR}/doc/Doxyfile @ONLY)
# run Doxygen in build directory using the generated Doxyfile
add_custom_target(doc
	${DOXYGEN_EXECUTABLE}
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/doc
	COMMENT "Generating dOpenCL API documentation")


#
# install configuration
#

install(TARGETS dcl
	RUNTIME DESTINATION lib
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib)

INSTALL( DIRECTORY ${PROJECT_SOURCE_DIR}/include/CL DESTINATION include )
INSTALL( DIRECTORY ${PROJECT_SOURCE_DIR}/include/dcl DESTINATION include )

# TODO Install dOpenCL API documentation
