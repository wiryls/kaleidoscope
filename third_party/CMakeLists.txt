
cmake_minimum_required(VERSION 3.14) # require 3.14 for FetchContent_MakeAvailable
include(FetchContent)                # see: https://cmake.org/cmake/help/latest/module/FetchContent.html
set(FETCHCONTENT_QUIET OFF)          # for debug

# third party libraries
#

# helper
# this part is used to collect all third party targets
# https://stackoverflow.com/a/25785856
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
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
endif()

# populate
FetchContent_MakeAvailable()

# ide friendly
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_target_properties(${third_party_targets} PROPERTIES FOLDER "third_party")
endif()
