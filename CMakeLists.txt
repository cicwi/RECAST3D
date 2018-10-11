cmake_minimum_required(VERSION 3.0)

include(FindPkgConfig)

set(TARGET_NAME "slicerecon")
project(slicerecon)

# --------------------------------------------------------------------------------------------
# ASTRA
# pkg_check_modules(ASTRA REQUIRED "astra")
set(ASTRA_PATH = "../astra-scm")
set(ASTRA_INCLUDE_DIRS "${ASTRA_PATH}/include" "${ASTRA_PATH}/cuda")
link_directories("$ENV{HOME}/scratch/code/tomography/astra-scm/build/linux/.libs/")

add_library(astra-toolbox INTERFACE)
target_include_directories(astra-toolbox SYSTEM INTERFACE ${ASTRA_INCLUDE_DIRS})
target_compile_definitions(astra-toolbox INTERFACE "ASTRA_CUDA")
target_link_libraries(astra-toolbox INTERFACE "astra")
# --------------------------------------------------------------------------------------------
# Eigen
find_package (Eigen3 3.3 QUIET NO_MODULE)
if (Eigen3_FOUND)
  message("Eigen found" ${EIGEN3_INCLUDE_DIR})
else()
  message("Eigen not found")
endif()
# --------------------------------------------------------------------------------------------
# ZeroMQ
find_package(ZeroMQ QUIET)

if (ZeroMQ_FOUND)
    add_library(zmq INTERFACE)
    target_include_directories(zmq INTERFACE ${ZeroMQ_INCLUDE_DIR})
    target_link_libraries(zmq INTERFACE ${ZeroMQ_LIBRARY})
else()
    message("'zmq' not installed on the system, building from source...")

    execute_process(COMMAND git submodule update --init --remote -- ext/libzmq
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    set(ZMQ_BUILD_TESTS OFF CACHE BOOL "disable tests" FORCE)
    set(WITH_PERF_TOOL OFF CACHE BOOL "disable perf-tools" FORCE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/ext/libzmq)
    set(ZMQ_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/ext/libzmq/include)

    # ZeroMQ names their target libzmq, which is inconsistent => create a ghost dependency
    add_library(zmq INTERFACE)
    target_link_libraries(zmq INTERFACE libzmq)
endif()
# --------------------------------------------------------------------------------------------
# cppzmq
execute_process(COMMAND git submodule update --init --remote -- ext/cppzmq
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_library(cppzmq INTERFACE)
target_include_directories(cppzmq INTERFACE ext/cppzmq)
target_link_libraries(cppzmq INTERFACE zmq)
# --------------------------------------------------------------------------------------------
# tomopackets
execute_process(COMMAND git submodule update --init --remote -- ext/tomopackets
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# TODO move to tomopackets
add_library(tomop INTERFACE)
target_include_directories(tomop INTERFACE ext/tomopackets/include)
target_link_libraries(tomop INTERFACE cppzmq zmq)
# --------------------------------------------------------------------------------------------
# Bulk
add_subdirectory(${CMAKE_SOURCE_DIR}/ext/bulk)
# --------------------------------------------------------------------------------------------

set(
    SOURCES
    "src/util/util.cpp"
    "src/util/log.cpp"
    "src/util/processing.cpp"
    "src/reconstruction/reconstructor.cpp"
    "src/reconstruction/helpers.cpp"
)

set(
  LIB_NAMES
  "astra-toolbox"
  "Eigen3::Eigen"
  "flags"
  "zmq"
  "cppzmq"
  "tomop"
  "bulk_thread"
)

add_library(${TARGET_NAME} ${SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC "include")
target_link_libraries(${TARGET_NAME} ${LIB_NAMES})
target_compile_options(${TARGET_NAME} PUBLIC "-Wfatal-errors" "-Werror" "-Wall" "-std=c++17")

add_subdirectory(example)
