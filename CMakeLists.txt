cmake_minimum_required(VERSION 3.0...3.15)

project(baglib LANGUAGES C CXX VERSION 2.0.0)

#needed when CMakeSettings.json is not available
#list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/../third_party/build;${CMAKE_SOURCE_DIR}/../third_party/build/lib" )


# Disable addition of warning flags to the CMAKE_<LANG>_FLAGS by default for MSVC.
if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()


# SWIG: use standard target name.
if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()

# SWIG: use SWIG_MODULE_NAME property.
#if(POLICY CMP0086)
#  cmake_policy(SET CMP0086 NEW)
#endif()



#-----------------------------------------------------------------------------
# Option to Build Shared/Static libs, default is shared
#-----------------------------------------------------------------------------

## turning this on causes unresolved external errors in swig-generated wrapper and pyd files - investigate cause.
#option(BUILD_SHARED_LIBS "Build Shared Libraries" ON)

add_subdirectory(api)

## turning this on produces warnings about conflicting names in python
#add_subdirectory(examples)

# Python bindings
option(BUILD_PYTHON "Build Python bindings" OFF)


## turning on testing produces errors about not finding the bag files?

# Testing
#option(BUILD_TESTS "Build Tests" ON)

#if(BUILD_TESTS)
#    add_subdirectory(tests)
#endif()

