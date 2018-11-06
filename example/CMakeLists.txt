cmake_minimum_required(VERSION 3.0)

# --------------------------------------------------------------------------------------------
# Flags
execute_process(COMMAND git submodule update --init -- ext/flags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_subdirectory("ext/flags")
# --------------------------------------------------------------------------------------------
# Tomos
execute_process(COMMAND git submodule update --init -- ext/tomos
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

set(TOMOS_LIB_ONLY ON CACHE BOOL "build Tomos as a library" FORCE)
add_subdirectory("ext/tomos")
# --------------------------------------------------------------------------------------------

add_executable(slicerecon_server "slicerecon_server.cpp")
target_link_libraries(slicerecon_server slicerecon flags)

add_executable(slicerecon_push_tomos "slicerecon_push_tomos.cpp")
target_link_libraries(slicerecon_push_tomos slicerecon flags tomos)
