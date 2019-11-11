# The name of our project is "openns". CMakeLists files in this project can
# refer to the root source directory of the project as ${OPENNS_SOURCE_DIR} and
# to the root binary directory of the project as ${OPENNS_BINARY_DIR}.
cmake_minimum_required(VERSION 3.0...3.15)
project(OPENNS LANGUAGES C CXX VERSION 2.0.0)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

#-----------------------------------------------------------------------------
# Define some CMake variables for use later in the project
#-----------------------------------------------------------------------------
set(OPENNS_SRC_DIR                             ${OPENNS_SOURCE_DIR}/api)
set(OPENNS_EXAMPLES_SOURCE_DIR                 ${OPENNS_SOURCE_DIR}/examples)

if(WIN32)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OPENNS_SOURCE_DIR}/bin)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${OPENNS_SOURCE_DIR}/../third_party/build/bin)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OPENNS_SOURCE_DIR}/lib)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG   ${OPENNS_SOURCE_DIR}/../third_party/build/lib)
endif()

#-----------------------------------------------------------------------------
# parse the full version number from bag.h and include in OPENNS_VERS_INFO
#-----------------------------------------------------------------------------
file(READ ${OPENNS_SRC_DIR}/bag_version.h _bag_version_h_contents)
string(REGEX REPLACE ".*#define[ \t]+BAG_VER_MAJOR[ \t]+([0-9]*).*$"
    "\\1" OPENNS_VERS_MAJOR ${_bag_version_h_contents})
string(REGEX REPLACE ".*#define[ \t]+BAG_VER_MINOR[ \t]+([0-9]*).*$"
    "\\1" OPENNS_VERS_MINOR ${_bag_version_h_contents})
string(REGEX REPLACE ".*#define[ \t]+BAG_VER_REVISION[ \t]+([0-9]*).*$"
    "\\1" OPENNS_VERS_RELEASE ${_bag_version_h_contents})

#-----------------------------------------------------------------------------
set(OPENNS_PACKAGE "openns")
set(OPENNS_PACKAGE_NAME "OPENNS")
set(OPENNS_PACKAGE_VERSION "${OPENNS_VERS_MAJOR}.${OPENNS_VERS_MINOR}")
set(OPENNS_PACKAGE_VERSION_MAJOR "${OPENNS_VERS_MAJOR}.${OPENNS_VERS_MINOR}")
set(OPENNS_PACKAGE_VERSION_MINOR "${OPENNS_VERS_RELEASE}")
set(OPENNS_PACKAGE_STRING "${OPENNS_PACKAGE_NAME} ${OPENNS_PACKAGE_VERSION}")
set(OPENNS_PACKAGE_TARNAME "bag")
set(OPENNS_PACKAGE_URL "http://www.opennavsurf.org")
set(VERSION 2.0.0 OPENNS_PACKAGE_VERSION)

#-----------------------------------------------------------------------------
# Option to Build Shared/Static libs, default is shared
#-----------------------------------------------------------------------------
option(BUILD_SHARED_LIBS "Build Shared Libraries" ON)

#-----------------------------------------------------------------------------
# Option to Build against Shared/Static version of HDF5, default is shared
#-----------------------------------------------------------------------------
option(BUILD_WITH_SHARED_HDF5 "Build with HDF5 as a shared library" ON)


if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "--std=gnu99 ${CMAKE_C_FLAGS}")
    add_definitions("-Wall")
endif()

if(NOT WIN32)
	# use, i.e. don't skip the full RPATH for the build tree
	set(CMAKE_SKIP_BUILD_RPATH  FALSE)

	# when building, don't use the install RPATH already
	# (but later on when installing)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

	#SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

	# add the automatically determined parts of the RPATH
	# which point to directories outside the build tree to the install RPATH
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

	# the RPATH to be used when installing, but only if it's not a system directory
	list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
	if("${isSystemDir}" STREQUAL "-1")
	   set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
	endif("${isSystemDir}" STREQUAL "-1")
endif()

# Recurse into the "api" and "examples" subdirectories. This does not actually
# cause another cmake executable to run. The same process will walk through
# the project's entire directory structure.
add_subdirectory(api)
add_subdirectory(examples)

# Testing
option(OPENNS_BUILD_TESTS "Build Tests" ON)

if(OPENNS_BUILD_TESTS)
    add_subdirectory(tests)
endif()

