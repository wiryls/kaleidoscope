# require 3.11 for FetchContent and 3.14 for FetchContent_MakeAvailable
# see: https://cmake.org/cmake/help/latest/module/FetchContent.html
cmake_minimum_required(VERSION 3.14)
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# third party libraries
#

# helper
# this part is used to collect all third party targets
# https://stackoverflow.com/a/25785856
set(third_party_targets "" CACHE INTERNAL "third party target list")
function(add_library name)
    _add_library(${name} ${ARGN})
    remember_target(${name})
endfunction()
function(add_executable name)
    _add_executable(${name} ${ARGN})
    remember_target(${name})
endfunction()
function(add_custom_target name)
    _add_custom_target(${name} ${ARGN})
    remember_target(${name})
endfunction()
function(remember_target name)
    if(TARGET ${name} AND NOT ${name} IN_LIST third_party_targets)
        get_target_property(_aliased  ${name} ALIASED_TARGET)
        get_target_property(_imported ${name} IMPORTED)
        get_target_property(_type     ${name} TYPE)
        if(NOT _aliased AND NOT _imported AND NOT ${type} STREQUAL "INTERFACE_LIBRARY")
            set(third_party_targets ${third_party_targets} ${name} CACHE INTERNAL "third party target list")
        endif()
    endif()
endfunction()

# populate
FetchContent_MakeAvailable("")
set_target_properties     (${third_party_targets} PROPERTIES FOLDER "third_party")
