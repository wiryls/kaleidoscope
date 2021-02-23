# shared options
add_compile_options(
    "$<$<CXX_COMPILER_ID:MSVC>:/WX;/W4;/utf-8>"
    "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g;-pedantic-errors;-Wall;-Wextra;-Wconversion;-Werror>")

# add all subprojects
# TODO:

# add optional example
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND KDENTICON_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
