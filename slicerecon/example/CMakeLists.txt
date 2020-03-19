cmake_minimum_required(VERSION 3.0)

# --------------------------------------------------------------------------------------------
# Flags
execute_process(COMMAND git submodule update --init -- ../../ext/flags
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory("../../ext/flags" flags)

# --------------------------------------------------------------------------------------------

add_executable(slicerecon_server "slicerecon_server.cpp")
target_link_libraries(slicerecon_server slicerecon flags)
