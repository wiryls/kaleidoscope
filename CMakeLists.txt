# cmake requirement
cmake_minimum_required(VERSION 3.14...3.19)

# project infomation
project(kdenticon
    VERSION 0.1
    DESCRIPTION "Yet an unfinished identicon generator"
    LANGUAGES CXX
)

# provide options for users
option(KDENTICON_BUILD_TESTS    "build all test programs"   ON)
option(KDENTICON_BUILD_EXAMPLES "build all sample programs" ON)

# set global properties or options
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "")

# add their and our libraries
add_subdirectory(third_party)
add_subdirectory(src)

# add executable
add_subdirectory(apps)

# add tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND KDENTICON_BUILD_TESTS)
    add_subdirectory(tests)
endif()
