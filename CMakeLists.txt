# CMake 3.15+ needed for SWIG version 4.0+ support.
cmake_minimum_required(VERSION 3.15)
project(OpenNavSurf-BAG LANGUAGES C CXX VERSION 2.0.1)

# SWIG: use standard target name.
if(POLICY CMP0078)
    cmake_policy(SET CMP0078 NEW)
endif()

# SWIG: use SWIG_MODULE_NAME property.
if(POLICY CMP0086)
    cmake_policy(SET CMP0086 NEW)
endif()

# Disable addition of warning flags to the CMAKE_<LANG>_FLAGS by default for MSVC.
if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

# Allow bag_version.h to be generated in CMake 3.20+ without getting a warning
# of a non-explicit source file.
if(POLICY CMP0115)
    cmake_policy(SET CMP0115 NEW)
endif()

# Options
option(BAG_BUILD_SHARED_LIBS "Build Shared Libraries" ON)
option(BAG_BUILD_PYTHON "Build Python bindings using SWIG" OFF)
option(BAG_BUILD_TESTS "Build Tests" OFF)
option(BAG_CODE_COVERAGE "Compute code coverage for C++ unit tests" OFF)
option(BAG_BUILD_DOCS "Build Documentation" OFF)
option(BAG_CI "The build is on the CI (GitHub Actions/Appveyor)" OFF)

# Put the artifacts into a single directory for MSVC so running tests is easier.
if(MSVC AND NOT BAG_CI)
    if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/stage/${CMAKE_INSTALL_BINDIR})
    endif()

    if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/stage/${CMAKE_INSTALL_LIBDIR})
    endif()

    if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/stage/${CMAKE_INSTALL_LIBDIR})
    endif()
endif()

if(BAG_CODE_COVERAGE)
    set(CODE_COVERAGE ON CACHE INTERNAL "Turn on code coverage")
    include(CMakeModules/code-coverage.cmake)
    # Exclude C wrapper for now since our tests only target C++ code currently
    add_code_coverage_all_targets(EXCLUDE test/* api/bag.cpp api/bag.h)
endif()


if(BAG_BUILD_PYTHON)
    set(BAG_BUILD_SWIG ON)
endif()

add_subdirectory(api)
add_subdirectory(examples)

if(BAG_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(BAG_BUILD_DOCS)
    set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules" ${CMAKE_MODULE_PATH})
    add_subdirectory(docs)
endif()
