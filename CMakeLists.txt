cmake_minimum_required (VERSION 2.8)
#SET(CMAKE_VERBOSE_MAKEFILE ON)

SET (CMAKE_CXX_FLAGS          "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
SET (CMAKE_CXX_FLAGS          "${CMAKE_CXX_FLAGS} -Wno-deprecated")

MESSAGE("C++ Compiler: ${CMAKE_CXX_COMPILER} Flags: ${CMAKE_CXX_FLAGS}")
project (json_wrapper)

# The version number.
set (JSON_VERSION_MAJOR 1)
set (JSON_VERSION_MINOR 0)

include_directories("/opt/local/include")
link_directories("/opt/local/lib")
SET (CMAKE_CXX_FLAGS          "${CMAKE_CXX_FLAGS} -ljansson")


# The project directories.
set (CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}")
set (PROJECT_BINARY_DIR "${CMAKE_CURRENT_LIST_DIR}/bin")
set (PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")
set (PROJECT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/impl")


# add the binary tree to the search path for include files
include_directories("${PROJECT_INCLUDE_DIR}") # find all other include files.

add_subdirectory(${PROJECT_INCLUDE_DIR})
add_subdirectory(${PROJECT_SOURCE_DIR})




