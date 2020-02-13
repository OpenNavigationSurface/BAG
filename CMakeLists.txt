cmake_minimum_required(VERSION 3.0...3.15)
project(OPENNS LANGUAGES C CXX VERSION 2.0.0)

# Disable addition of warning flags to the CMAKE_<LANG>_FLAGS by default for MSVC.
if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

# SWIG: use standard target name.
if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()

#-----------------------------------------------------------------------------
# Option to Build Shared/Static libs, default is shared
#-----------------------------------------------------------------------------
option(BUILD_SHARED_LIBS "Build Shared Libraries" ON)

add_subdirectory(api)
add_subdirectory(examples)

# Python bindings
option(BUILD_PYTHON "Build Python bindings" OFF)

# Testing
option(BUILD_TESTS "Build Tests" ON)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

