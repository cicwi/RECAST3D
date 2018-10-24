cmake_minimum_required(VERSION 3.0.0)
include(FindPkgConfig)

project(tomopackets)

include_directories(
    "include"
)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wfatal-errors -Wextra -g -O3 -std=c++14")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(EXEC_NAME "test_server")

set(
    SOURCE_NAMES
    "test/test_server.cpp"
)

find_package(ZeroMQ REQUIRED)
find_package(cppzmq REQUIRED)

set(
    LIB_NAMES
    "libzmq"
    "cppzmq"
    "pthread"
)

add_executable(${EXEC_NAME} ${SOURCE_NAMES})
target_link_libraries(${EXEC_NAME} ${LIB_NAMES})

add_subdirectory("ext/pybind11")

set(BINDING_NAME "py_tomop")
set(BINDING_SOURCES "tomop/module.cpp")
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "tomop")

pybind11_add_module(${BINDING_NAME} ${BINDING_SOURCES})

target_link_libraries(${BINDING_NAME} PRIVATE ${LIB_NAMES})
