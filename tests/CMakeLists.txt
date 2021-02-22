# require 3.14 for FetchContent_MakeAvailable
cmake_minimum_required(VERSION 3.14)

# settings for tests
enable_testing()
# prevent CTest added targets
# see: https://stackoverflow.com/questions/44949354
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

#############################################################################
# dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET ON)

# catch2 for unit test
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.0-preview3)

# enable all dependencies
FetchContent_MakeAvailable(Catch2)
set_target_properties     (Catch2 Catch2WithMain PROPERTIES FOLDER "third_party/tests")

#############################################################################
# subprojects

# shared settings for subprojects
add_compile_options(
    "$<$<CXX_COMPILER_ID:MSVC>:/WX;/W4;/utf-8>"
    "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g;-pedantic-errors;-Wall;-Wextra;-Wconversion;-Werror>")

# add all subprojects
add_subdirectory(core_tests)
