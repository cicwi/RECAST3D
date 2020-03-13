cmake_minimum_required(VERSION 3.0.0)
include(FindPkgConfig)

project(tomopackets)

option(TOMOP_LIB_ONLY "Do not build python bindings")

include_directories(
    "include"
)

set(CXX_FLAGS "-Wall" "-Wfatal-errors" "-Wextra" "-g" "-O3" "-std=c++14")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(EXEC_NAME "test_server")

set(
    SOURCE_NAMES
    "test/test_server.cpp"
)

# --------------------------------------------------------------------------------------------
# ZeroMQ version 4.X
if (NOT TARGET zmq)
find_package(ZeroMQ 4 QUIET)

if (ZeroMQ_FOUND)
    add_library(zmq INTERFACE)
    target_include_directories(zmq INTERFACE ${ZeroMQ_INCLUDE_DIR})
    target_link_libraries(zmq INTERFACE ${ZeroMQ_LIBRARY})
else()
    message("'zmq' not installed on the system, building from source...")

    execute_process(COMMAND git submodule update --init -- ext/libzmq
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    set(ZMQ_BUILD_TESTS OFF CACHE BOOL "disable tests" FORCE)
    set(WITH_PERF_TOOL OFF CACHE BOOL "disable perf-tools" FORCE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/ext/libzmq)
    set(ZMQ_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/ext/libzmq/include)

    # ZeroMQ names their target libzmq, which is inconsistent => create a ghost dependency
    add_library(zmq INTERFACE)
    target_link_libraries(zmq INTERFACE libzmq)
endif()
endif()
# --------------------------------------------------------------------------------------------
# cppzmq

find_package(cppzmq QUIET)
if (NOT cppzmq_FOUND AND NOT TARGET cppzmq)
  execute_process(COMMAND git submodule update --init -- ext/cppzmq
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

  add_library(cppzmq INTERFACE)
  target_include_directories(cppzmq INTERFACE ext/cppzmq)
  target_link_libraries(cppzmq INTERFACE zmq)
endif()

# --------------------------------------------------------------------------------------------
# Boost
find_package(Boost 1.61 REQUIRED)
# --------------------------------------------------------------------------------------------
set(
    LIB_NAMES
    "zmq"
    "cppzmq"
    "pthread"
    "Boost::boost"
)

if (NOT TOMOP_LIB_ONLY)
add_executable(${EXEC_NAME} ${SOURCE_NAMES})
target_link_libraries(${EXEC_NAME} ${LIB_NAMES})
target_compile_options(${EXEC_NAME} PRIVATE ${CXX_FLAGS})

add_subdirectory("ext/pybind11")

set(BINDING_NAME "py_tomop")
set(BINDING_SOURCES "tomop/module.cpp")

pybind11_add_module(${BINDING_NAME} ${BINDING_SOURCES})
target_link_libraries(${BINDING_NAME} PRIVATE ${LIB_NAMES})
target_compile_options(${BINDING_NAME} PRIVATE ${CXX_FLAGS})
endif()

add_library(tomop INTERFACE)
target_include_directories(tomop INTERFACE "include")
target_link_libraries(tomop INTERFACE cppzmq zmq)
