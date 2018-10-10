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

add_subdirectory("ext/tomos")
# --------------------------------------------------------------------------------------------

add_executable(setting_up_server "setting_up_server.cpp")
target_link_libraries(setting_up_server slicerecon flags)

add_executable(push_projections "push_projections.cpp")
target_link_libraries(push_projections slicerecon flags tomos)
