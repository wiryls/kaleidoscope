# configuration
# require 3.14 for FetchContent_MakeAvailable
cmake_minimum_required(VERSION 3.14)
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# backward
FetchContent_Declare(
    backward
    GIT_REPOSITORY https://github.com/bombela/backward-cpp
    GIT_TAG        v1.6
    GIT_SHALLOW    true)

# populate
FetchContent_MakeAvailable(backward)
