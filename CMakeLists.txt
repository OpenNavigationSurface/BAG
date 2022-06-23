cmake_minimum_required(VERSION 3.15)

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

project(OPENNS LANGUAGES C CXX VERSION 2.0.0)

# Options
option(BAG_BUILD_SHARED_LIBS "Build Shared Libraries" ON)
option(BAG_BUILD_SWIG "Build SWIG interface" OFF)
option(BAG_BUILD_PYTHON "Build Python bindings" OFF)
option(BAG_CI "The build is on the CI (Travis/Appveyor)." OFF)

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

add_subdirectory(api)
add_subdirectory(examples)

option(BAG_BUILD_TESTS "Build Tests" ON)

if(BAG_BUILD_TESTS)
    add_subdirectory(tests)
endif()

