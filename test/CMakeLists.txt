cmake_minimum_required(VERSION 3.10)

project(dOpenCLtest)

set(DOPENCL_INCLUDE_DIR "${dOpenCLlib_SOURCE_DIR}/include" CACHE PATH "Path to dOpenCL headers")
set(DOPENCL_LIBRARY_DIR "${dOpenCLlib_BINARY_DIR}" CACHE PATH "Path to dOpenCL library")
set(DOPENCL_ICD_DIR "${dOpenCLicd_BINARY_DIR}" CACHE PATH "Path to dOpenCL ICD")

# search for Boost.Test
find_package(Boost 1.41.0 COMPONENTS unit_test_framework REQUIRED)

include_directories(${DOPENCL_INCLUDE_DIR}
                    ${Boost_INCLUDE_DIRS})

link_directories(${DOPENCL_ICD_DIR}
                 ${DOPENCL_LIBRARY_DIR}
                 ${Boost_LIBRARY_DIRS})

# generate dOpenCL node file
file(WRITE ${PROJECT_BINARY_DIR}/dcl.nodes
	"localhost")

# 
# dOpenCL test targets
#
# enable_testing() must be defined in the root CMakeLists.txt, otherwise 'make
# test' will not work correctly

set(UTILITY_SOURCES ${PROJECT_SOURCE_DIR}/src/utility.cpp)

add_executable(CommandQueue ${UTILITY_SOURCES} ${PROJECT_SOURCE_DIR}/src/CommandQueue.cpp)
add_executable(Context ${UTILITY_SOURCES} ${PROJECT_SOURCE_DIR}/src/Context.cpp)
add_executable(Device ${UTILITY_SOURCES} ${PROJECT_SOURCE_DIR}/src/Device.cpp)
add_executable(Event ${UTILITY_SOURCES} ${PROJECT_SOURCE_DIR}/src/Event.cpp)
add_executable(Logger ${UTILITY_SOURCES} ${PROJECT_SOURCE_DIR}/src/Logger.cpp)
add_executable(Memory
		${UTILITY_SOURCES}
		${PROJECT_SOURCE_DIR}/src/Buffer.cpp
		${PROJECT_SOURCE_DIR}/src/Mapping.cpp
		${PROJECT_SOURCE_DIR}/src/Memory.cpp
		${PROJECT_SOURCE_DIR}/src/MemoryConsistency.cpp)
add_executable(Platform ${PROJECT_SOURCE_DIR}/src/Platform.cpp)
add_executable(Program ${UTILITY_SOURCES} ${PROJECT_SOURCE_DIR}/src/Program.cpp)
add_executable(DataTransfer ${UTILITY_SOURCES} ${PROJECT_SOURCE_DIR}/src/DataTransfer.cpp)

foreach(test CommandQueue Context Device Event Logger Memory Platform Program DataTransfer)
	add_test(${test} ${test})
	
	target_link_libraries(${test}
		dOpenCL
		dcl
		${Boost_LIBRARIES})
		
	target_compile_definitions(${test} PRIVATE BOOST_TEST_DYN_LINK)
	target_compile_definitions(${test} PRIVATE CL_TARGET_OPENCL_VERSION=120
	                                           CL_HPP_MINIMUM_OPENCL_VERSION=120
	                                           CL_HPP_TARGET_OPENCL_VERSION=120
	                                           CL_HPP_ENABLE_EXCEPTIONS)
endforeach(test)
