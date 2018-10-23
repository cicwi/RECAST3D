set(BINDING_NAME "py_slicerecon")
set(BINDING_SOURCES "src/module.cpp")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

pybind11_add_module(${BINDING_NAME} ${BINDING_SOURCES})

target_link_libraries(${BINDING_NAME} PRIVATE slicerecon)
