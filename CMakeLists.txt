cmake_minimum_required(VERSION 3.28)

project(tinyply
    LANGUAGES CXX
)
set(PROJECT_VERSION 2.3)

# Compiler requirements
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/tinyply")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/third-party")

if(NOT CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX "d")
endif()

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Library
option(LIB "Build as library" ON)
if(LIB)
    add_subdirectory(source)
endif()

add_subdirectory(examples)

# Tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(BUILD_TESTS "Build tests" OFF)
    if(BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()
