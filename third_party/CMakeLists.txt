# configuration
# require 3.25 for FetchContent_Declare(SYSTEM)
cmake_minimum_required(VERSION 3.25)
include(FetchContent)

# backward
FetchContent_Declare(
    backward
    GIT_REPOSITORY https://github.com/bombela/backward-cpp
    GIT_TAG        v1.6
    GIT_SHALLOW    true
    SYSTEM)

# populate
FetchContent_MakeAvailable(backward)
