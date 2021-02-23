# use dir name as name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# source lists
set(sources
    test_archive.cpp
    test_cast.cpp
    test_graphic.cpp
    test_martix.cpp
    test_scalar.cpp)

# executable
add_executable         (${name} ${sources})
target_link_libraries  (${name} PRIVATE Catch2::Catch2WithMain kdenticon)
target_compile_features(${name} PRIVATE cxx_std_17)

# add for CTest and ide friendly
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_test             (NAME ${name} COMMAND ${name})
    set_target_properties(${name} PROPERTIES FOLDER "${PROJECT_NAME}/tests")
endif()
