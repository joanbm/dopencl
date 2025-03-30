cmake_minimum_required(VERSION 3.10)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})

project(dOpenCL)

# Enable C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Keep GNU extensions enabled (default), in order to simplify the build on
# PowerPC due to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=58241.
# An alternative is to disable Altivec (-mno-altivec), check the history of
# this file in version control for details
#set(CMAKE_CXX_EXTENSIONS OFF)

# Avoid 'ignoring attributes on template argument' spam because of types such as std::vector<cl_int>
# See https://github.com/KhronosGroup/OpenCL-CLHPP/issues/37 for more information
if (CMAKE_COMPILER_IS_GNUCXX AND ((CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0) OR (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 6.0)))
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-attributes")
endif()

option(BUILD_UNIT_TESTS "Build dOpenCL unit tests (experimental)" OFF)
option(ENABLE_IO_LINK_COMPRESSION "Enable I/O Link Compression (using lib842) for transfers" ON)
option(USE_HW_IO_LINK_COMPRESSION "Use in-kernel and potentially hardware-accelerated 842 implementation" ON)
option(USE_CL_IO_LINK_COMPRESSION "Use OpenCL GPU-accelerated 842 implementation implementation" ON)

#
# dOpenCL version information
#
# Note that the version number refers to the dOpenCL ICD, daemon, and the
# dOpenCL API. The dOpenCL library may have a different version number.
#
set(DCL_VERSION_MAJOR 0)
set(DCL_VERSION_MINOR 4)
set(DCL_VERSION_PATCH 0)
set(DCL_VERSION ${DCL_VERSION_MAJOR}.${DCL_VERSION_MINOR}.${DCL_VERSION_PATCH})

if (ENABLE_IO_LINK_COMPRESSION)
	add_subdirectory (lib842)
	add_definitions(-DIO_LINK_COMPRESSION)
	if (USE_HW_IO_LINK_COMPRESSION)
		add_definitions(-DUSE_HW_IO_LINK_COMPRESSION)
	endif(USE_HW_IO_LINK_COMPRESSION)
	if (USE_CL_IO_LINK_COMPRESSION)
		add_definitions(-DUSE_CL_IO_LINK_COMPRESSION)
	endif(USE_CL_IO_LINK_COMPRESSION)
endif(ENABLE_IO_LINK_COMPRESSION)
add_subdirectory (dclasio)
add_subdirectory (daemon)
add_subdirectory (icdpp)

if(BUILD_UNIT_TESTS)
	# enable_testing must be defined in the root CMakeLists.txt, otherwise
	# 'make test' will not work correctly
	enable_testing ()
	add_subdirectory (test)
	add_subdirectory (standalone_test)
endif(BUILD_UNIT_TESTS)

# TODO Export targets from installation rather than build tree
#export(TARGETS dcl dOpenCL dcld FILE dOpenCL.cmake)


#
# package configuration
#
# TODO Add 'README_...txt' and 'INSTALL_...txt' to package
# TODO Add copyright and license header to all source files
# TODO Add dOpenCL API headers to package
# TODO Add dOpenCL API documentation to package

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "dOpenCL is a distributed implementation of the OpenCL API.")
set(CPACK_PACKAGE_VENDOR "Group Parallel and Distributed Systems, Department of Computer Science, University of Muenster, Germany")
set(CPACK_PACKAGE_COPYRIGHT_FILE "${PROJECT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR ${DCL_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${DCL_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${DCL_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "dOpenCL-${DCL_VERSION_MAJOR}.${DCL_VERSION_MINOR}")
set(CPACK_PACKAGE_FILE_NAME "dOpenCL-${DCL_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_TYPE}")

# WARNING:
# CPack TGZ generator will include *all* files from the source folder by default
# Checkout a clean tree from the repository to create a source package.
set(CPACK_SOURCE_IGNORE_FILES
	"/build;/.svn/;.*~$;${CPACK_SOURCE_IGNORE_FILES}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "dOpenCL-${DCL_VERSION}-src")
set(CPACK_SOURCE_STRIP_FILES TRUE)

if(UNIX)
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README_Linux.txt")
set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
endif(UNIX)
if(WIN32)
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README_Win.txt")
set(CPACK_GENERATOR "ZIP")
set(CPACK_SOURCE_GENERATOR "ZIP")
endif(WIN32)

# CPack must be included *after* the CPack configuration
include(CPack)

