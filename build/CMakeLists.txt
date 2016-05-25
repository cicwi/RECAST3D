cmake_minimum_required(VERSION 3.0.0)
include(FindPkgConfig)

project(tomovis)

include_directories(
    "../include"
    "../ext/imgui/"
    "../ext/imgui/examples/libs/gl3w/"
    "../ext/imgui/examples/opengl3_example/"
)

# link_directories(
#     "../ext/submodule/lib"
# )

pkg_check_modules(GLFW glfw3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wfatal-errors -Wextra -g -O3")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(EXEC_NAME "tomovis")

set(
    SOURCE_NAMES
    "../ext/imgui/imgui.cpp"
    "../ext/imgui/imgui_draw.cpp"
    "../ext/imgui/imgui_demo.cpp"
    "../ext/imgui/examples/opengl3_example/imgui_impl_glfw_gl3.cpp"
    "../ext/imgui/examples/libs/gl3w/GL/gl3w.c"
    "../src/main.cpp"
)

MESSAGE(STATUS "GLFW_STATIC_LIBRARIES: " ${GLFW_STATIC_LIBRARIES})

set(
    LIB_NAMES
    "GL"
    ${GLFW_STATIC_LIBRARIES}
)

## multiple binaries:
# foreach(source_file ${EXAMPLE_SOURCES})
#     string(REPLACE ".cpp" "" source_name ${source_file})
#     add_executable(${source_name} ${EXAMPLE_DIR}${source_file})
#     target_link_libraries( ${source_name} ${LIB_NAMES} )
# endforeach(source_file)

add_executable(${EXEC_NAME} ${SOURCE_NAMES})
target_link_libraries(${EXEC_NAME} ${LIB_NAMES} )

add_subdirectory(../test/build ../test/build)
