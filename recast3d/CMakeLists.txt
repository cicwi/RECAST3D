cmake_minimum_required(VERSION 3.0.0)
include(FindPkgConfig)

project(recast3d)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(EXEC_NAME "recast3d")

# multiple dependencies define 'uninstall' target
set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)

## DEPENDENCIES
# --------------------------------------------------------------------------------------------
# GLM
execute_process(COMMAND git submodule update --init -- ext/glm
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
add_subdirectory(${PROJECT_SOURCE_DIR}/../ext/glm glm)

# GLFW
execute_process(COMMAND git submodule update --init -- ext/glfw
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/..)

add_subdirectory(${PROJECT_SOURCE_DIR}/../ext/glfw glfw)
set(glfw3_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../ext/glfw/include)

# --------------------------------------------------------------------------------------------
# ImGui
execute_process(COMMAND git submodule update --init -- ext/imgui
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/..)

set(
    IMGUI_SOURCE_NAMES
    "${PROJECT_SOURCE_DIR}/../ext/imgui/imgui.cpp"
    "${PROJECT_SOURCE_DIR}/../ext/imgui/imgui_draw.cpp"
    "${PROJECT_SOURCE_DIR}/../ext/imgui/imgui_widgets.cpp"
    "${PROJECT_SOURCE_DIR}/../ext/imgui/examples/imgui_impl_opengl3.cpp"
    "${PROJECT_SOURCE_DIR}/../ext/imgui/examples/imgui_impl_glfw.cpp"
    "${PROJECT_SOURCE_DIR}/../ext/imgui/examples/libs/gl3w/GL/gl3w.c"
)

# ignore warnings from imgui
set_source_files_properties(
  ${IMGUI_SOURCE_NAMES}
  PROPERTIES
  COMPILE_FLAGS "-w"
)

add_library(imgui ${IMGUI_SOURCE_NAMES})
target_link_libraries(imgui PUBLIC glfw)
target_link_libraries(imgui PUBLIC dl)
target_include_directories(imgui PUBLIC
    "../ext/imgui"
    "../ext/imgui/examples/libs/gl3w/"
    "../ext/imgui/examples/opengl3_example/")

# --------------------------------------------------------------------------------------------
# ZeroMQ version 4.X
find_package(ZeroMQ 4 QUIET)

if (ZeroMQ_FOUND AND NOT TARGET zmq)
    add_library(zmq INTERFACE)
    target_include_directories(zmq INTERFACE ${ZeroMQ_INCLUDE_DIR})
    target_link_libraries(zmq INTERFACE ${ZeroMQ_LIBRARY})
else()
    message("'zmq' not installed on the system, building from source...")

    execute_process(COMMAND git submodule update --init -- ext/libzmq
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/..)

    set(ZMQ_BUILD_TESTS OFF CACHE BOOL "disable tests" FORCE)
    set(WITH_PERF_TOOL OFF CACHE BOOL "disable perf-tools" FORCE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/../ext/libzmq libzmq)
    set(ZMQ_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../ext/libzmq/include)

    # ZeroMQ names their target libzmq, which is inconsistent => create a ghost dependency
    add_library(zmq INTERFACE)
    target_link_libraries(zmq INTERFACE libzmq)
endif()

# --------------------------------------------------------------------------------------------
# OpenGL
find_package(OpenGL)

# --------------------------------------------------------------------------------------------
# Eigen

find_package (Eigen3 3.3 QUIET NO_MODULE)
if (Eigen3_FOUND)
  message("Eigen found" ${EIGEN3_INCLUDE_DIR})
else()
  message("Eigen not found, building from source")
  execute_process(COMMAND git submodule update --init -- ext/eigen
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/..)
  add_subdirectory(${PROJECT_SOURCE_DIR}/../ext/eigen eigen)
endif()

# --------------------------------------------------------------------------------------------
# cppzmq

find_package(cppzmq QUIET)
if (NOT cppzmq_FOUND)
  execute_process(COMMAND git submodule update --init -- ext/cppzmq
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/..)

  add_library(cppzmq INTERFACE)
  target_include_directories(cppzmq INTERFACE ../ext/cppzmq)
  target_link_libraries(cppzmq INTERFACE zmq)
endif()

# --------------------------------------------------------------------------------------------
# tomopackets

set(TOMOP_LIB_ONLY ON CACHE BOOL "build tomop only as a library" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/../tomopackets tomopackets)

# --------------------------------------------------------------------------------------------

set(
    SOURCE_NAMES
    "src/main.cpp"
    "src/math_common.cpp"
    "src/input.cpp"
    "src/util.cpp"
    "src/scene.cpp"
    "src/path.cpp"
    "src/scene_list.cpp"
    "src/server/server.cpp"
    "src/graphics/primitives.cpp"
    "src/graphics/scene_object.cpp"
    "src/graphics/scene_object_2d.cpp"
    "src/graphics/scene_object_3d.cpp"
    "src/graphics/scene_camera.cpp"
    "src/graphics/scene_camera_2d.cpp"
    "src/graphics/scene_camera_3d.cpp"
    "src/graphics/shader_program.cpp"
    "src/graphics/slice.cpp"
    "src/graphics/renderer.cpp"
    "src/graphics/components/reconstruction_component.cpp"
    "src/graphics/components/geometry_component.cpp"
    "src/graphics/components/axes_component.cpp"
    "src/graphics/components/control_component.cpp"
    "src/graphics/components/partitioning_component.cpp"
    "src/graphics/components/movie/recorder.cpp"
    "src/graphics/interface/scene_control.cpp"
    "src/graphics/interface/scene_switcher.cpp"
    "src/graphics/interface/window.cpp"
    "src/graphics/interface/interface.cpp"
)

set(
    LIB_NAMES
    "dl"
    "glfw"
    "X11"
    "pthread"
    "stdc++fs"
    "imgui"
    "zmq"
    "tomop"
    "glm"
    "Eigen3::Eigen"
)

add_executable(${EXEC_NAME} ${SOURCE_NAMES})
target_link_libraries(${EXEC_NAME} ${LIB_NAMES} ${OPENGL_gl_LIBRARY})

target_compile_options(${EXEC_NAME} PRIVATE -std=c++17 -Wall -Wfatal-errors -Wextra -g -O3)
# This is to work around GLM issue#754 <https://github.com/g-truc/glm/issues/754>
target_compile_definitions(${EXEC_NAME} PRIVATE -DGLM_ENABLE_EXPERIMENTAL -DGLM_FORCE_CTOR_INIT)

target_include_directories(${EXEC_NAME} PRIVATE
    "include"
)

target_include_directories(${EXEC_NAME} SYSTEM PRIVATE
    ${ASSIMP_INCLUDE_DIRS}
    ${ZeroMQ_INCLUDE_DIR}
)

# INSTALL COMMANDS
install(TARGETS recast3d imgui
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)

#add_subdirectory(test/build test/build)
