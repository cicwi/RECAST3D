cmake_minimum_required(VERSION 3.0.0)
include(FindPkgConfig)

project(tomopackets)

include_directories(
    "include"
    "ext/cppzmq/"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wfatal-errors -Wextra -g -O3")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(EXEC_NAME "test_server")

set(
    SOURCE_NAMES
    "test/test_server.cpp"
)

link_directories("ext/libzmq/build/lib/")

set(
    LIB_NAMES
    "zmq"
    "pthread"
)

add_executable(${EXEC_NAME} ${SOURCE_NAMES})
target_link_libraries(${EXEC_NAME} ${LIB_NAMES})
