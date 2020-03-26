cmake_minimum_required(VERSION 3.0)

include(FindPkgConfig)

set(TARGET_NAME "slicerecon")
project(slicerecon)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# --------------------------------------------------------------------------------------------
# GLM

find_package(glm QUIET)
if (glm_FOUND)
  message("glm found")
else()
    message("glm not found, including from source")
  execute_process(COMMAND git submodule update --init -- ext/glm
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  add_subdirectory(${PROJECT_SOURCE_DIR}/../ext/glm glm)
endif()

# --------------------------------------------------------------------------------------------
# ASTRA
pkg_check_modules(ASTRA REQUIRED "libastra")

add_library(astra-toolbox INTERFACE)
target_include_directories(astra-toolbox SYSTEM INTERFACE ${ASTRA_INCLUDE_DIRS})
target_compile_definitions(astra-toolbox INTERFACE "ASTRA_CUDA")
target_link_libraries(astra-toolbox INTERFACE ${ASTRA_LDFLAGS})
# --------------------------------------------------------------------------------------------
# Eigen
find_package (Eigen3 3.3 QUIET NO_MODULE)
if (Eigen3_FOUND)
  message("Eigen found" ${EIGEN3_INCLUDE_DIR})
else()
  message("Eigen not found, building from source")
  execute_process(COMMAND git submodule update --init -- ext/eigen
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  add_subdirectory(${PROJECT_SOURCE_DIR}/../ext/eigen)
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

    execute_process(COMMAND git submodule update --init -- ../ext/libzmq
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

    set(ZMQ_BUILD_TESTS OFF CACHE BOOL "disable tests" FORCE)
    set(WITH_PERF_TOOL OFF CACHE BOOL "disable perf-tools" FORCE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/../ext/libzmq libzmq)
    set(ZMQ_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../ext/libzmq/include)

    # ZeroMQ names their target libzmq, which is inconsistent => create a ghost dependency
    add_library(zmq INTERFACE)
    target_link_libraries(zmq INTERFACE libzmq)
endif()
# --------------------------------------------------------------------------------------------
# cppzmq

find_package(cppzmq QUIET)
if (NOT cppzmq_FOUND)
  execute_process(COMMAND git submodule update --init -- ../ext/cppzmq
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

  add_library(cppzmq INTERFACE)
  target_include_directories(cppzmq INTERFACE ../ext/cppzmq)
  target_link_libraries(cppzmq INTERFACE zmq)
endif()

# --------------------------------------------------------------------------------------------
# tomopackets
set(TOMOP_LIB_ONLY ON CACHE BOOL "build tomopackets as a library" FORCE)
add_subdirectory(../tomopackets tomopackets)
# --------------------------------------------------------------------------------------------
# Bulk
add_subdirectory(${PROJECT_SOURCE_DIR}/../ext/bulk bulk)
# --------------------------------------------------------------------------------------------
# FFTW3
find_package(FFTW REQUIRED)
# --------------------------------------------------------------------------------------------

set(
    SOURCES
    "src/util/util.cpp"
    "src/util/log.cpp"
    "src/util/bench.cpp"
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
  "glm"
  "fftw3f"
)

add_library(${TARGET_NAME} ${SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC "include")
target_link_libraries(${TARGET_NAME} ${LIB_NAMES})
target_compile_options(${TARGET_NAME} PUBLIC
    "-Wfatal-errors"
    "-Wall"
    "-O3"
    "-std=c++17"
    "-fPIC"
    "-static")


# --------------------------------------------------------------------------------------------
# Flags
execute_process(COMMAND git submodule update --init -- ext/flags
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory("../ext/flags" flags)

# --------------------------------------------------------------------------------------------

add_executable(slicerecon_server "src/slicerecon_server.cpp")
target_link_libraries(slicerecon_server slicerecon flags)

add_subdirectory("../ext/pybind11" pybind11)
add_subdirectory("python")

# INSTALL COMMANDS
install(TARGETS slicerecon_server
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)


