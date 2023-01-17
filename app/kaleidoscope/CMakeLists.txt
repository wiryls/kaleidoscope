# set name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# collect source
set(source main.cc)
set(header)

# executable
add_executable        (${name} ${source} ${header} ${BACKWARD_ENABLE})
add_backward          (${name})
set_target_properties (${name} PROPERTIES FOLDER "${PROJECT_NAME}")
set_target_properties (${name} PROPERTIES CXX_STANDARD 23)
set_target_properties (${name} PROPERTIES WIN32_EXECUTABLE ON)
target_link_libraries (${name} PRIVATE d3d11 dxgi)
target_compile_options(${name} PRIVATE
    "$<$<CXX_COMPILER_ID:MSVC>:/WX;/W4;/utf-8>"
    "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RELEASE>>:/O2>")
