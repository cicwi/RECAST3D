cmake_minimum_required(VERSION 3.0.0)
include(FindPkgConfig)

project(slicevis)

include_directories(
    "../include"
    "../ext/imgui/"
    "../ext/cppzmq/"
    "../ext/libzmq/include/"
    "../ext/assimp/include"
    "../ext/imgui/examples/libs/gl3w/"
    "../ext/imgui/examples/opengl3_example/"
)

link_directories(
    "~/.local/lib/"
    "../ext/libzmq/build/lib/"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wfatal-errors -Wextra -Werror -g -O3")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(EXEC_NAME "slicevis")

set(
    IMGUI_SOURCE_NAMES
    "../ext/imgui/imgui.cpp"
    "../ext/imgui/imgui_draw.cpp"
    "../ext/imgui/imgui_demo.cpp"
    "../ext/imgui/examples/opengl3_example/imgui_impl_glfw_gl3.cpp"
    "../ext/imgui/examples/libs/gl3w/GL/gl3w.c"
)

# ignore warnings from external libs
set_source_files_properties(
  ${IMGUI_SOURCE_NAMES}
  PROPERTIES
  COMPILE_FLAGS "-w"
)

set(
    SOURCE_NAMES
    ${IMGUI_SOURCE_NAMES}
    "../src/main.cpp"
    "../src/input.cpp"
    "../src/scene.cpp"
    "../src/scene_list.cpp"
    "../src/server/server.cpp"
    "../src/graphics/scene_object.cpp"
    "../src/graphics/scene_object_2d.cpp"
    "../src/graphics/scene_object_3d.cpp"
    "../src/graphics/scene_camera.cpp"
    "../src/graphics/scene_camera_2d.cpp"
    "../src/graphics/scene_camera_3d.cpp"
    "../src/graphics/shader_program.cpp"
    "../src/graphics/renderer.cpp"
    "../src/graphics/slice.cpp"
    "../src/graphics/components/reconstruction_component.cpp"
    "../src/graphics/components/geometry_component.cpp"
    "../src/graphics/components/mesh_component.cpp"
    "../src/graphics/components/movie_component.cpp"
    "../src/graphics/components/partitioning_component.cpp"
    "../src/graphics/interface/scene_control.cpp"
    "../src/graphics/interface/scene_switcher.cpp"
    "../src/graphics/interface/window.cpp"
    "../src/graphics/interface/interface.cpp"
)

set(
    EXAMPLE_NAME
    "../examples/client.cpp"
)



link_directories("../ext/zmq/lib")

set(
    LIB_NAMES
    "dl"
    "glfw"
    "GL"
    "zmq"
    "pthread"
    "assimp"
)

## multiple binaries:
# foreach(source_file ${EXAMPLE_SOURCES})
#     string(REPLACE ".cpp" "" source_name ${source_file})
#     add_executable(${source_name} ${EXAMPLE_DIR}${source_file})
#     target_link_libraries( ${source_name} ${LIB_NAMES} )
# endforeach(source_file)

add_executable(${EXEC_NAME} ${SOURCE_NAMES})
target_link_libraries(${EXEC_NAME} ${LIB_NAMES})

add_executable(client ${EXAMPLE_NAME})
target_link_libraries(client "zmq" "pthread")

add_subdirectory(../test/build ../test/build)
