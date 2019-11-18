# The name of our project is "openns". CMakeLists files in this project can
# refer to the root source directory of the project as ${OPENNS_SOURCE_DIR} and
# to the root binary directory of the project as ${OPENNS_BINARY_DIR}.
cmake_minimum_required(VERSION 3.0...3.15)
project(OPENNS LANGUAGES C CXX VERSION 2.0.0)

# Add comment
if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

#-----------------------------------------------------------------------------
# Option to Build Shared/Static libs, default is shared
#-----------------------------------------------------------------------------
# check whether we can remove the MACROs
option(BUILD_SHARED_LIBS "Build Shared Libraries" ON)

#-----------------------------------------------------------------------------
# Option to Build against Shared/Static version of HDF5, default is shared
#-----------------------------------------------------------------------------
# this should be based on BUILD_SHARED_LIBS
option(BUILD_WITH_SHARED_HDF5 "Build with HDF5 as a shared library" ON)

# Recurse into the "api" and "examples" subdirectories. This does not actually
# cause another cmake executable to run. The same process will walk through
# the project's entire directory structure.
add_subdirectory(api)
add_subdirectory(examples)

# Testing
option(BUILD_TESTS "Build Tests" ON)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

