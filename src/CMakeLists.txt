# set the target name
set(name "kdenticon")

# collect all header files and source files
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.hpp")
set(sources "")

# build as a library
add_library(${name} INTERFACE ${sources} ${headers})

# target options
set_target_properties     (${name} PROPERTIES FOLDER "${PROJECT_NAME}")
source_group              (TREE "${PROJECT_SOURCE_DIR}" FILES ${sources} ${headers})
target_include_directories(${name} INTERFACE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries     (${name} INTERFACE "")
target_compile_features   (${name} INTERFACE cxx_std_17)
target_compile_options    (${name} INTERFACE
    "$<$<CXX_COMPILER_ID:MSVC>:/WX;/W4;/utf-8>"
    "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g;-pedantic-errors;-Wall;-Wextra;-Wconversion;-Werror>")
