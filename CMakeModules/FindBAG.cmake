# Distributed under the OSI-approved BSD 3-Clause License.  See LICENSE
# file accompanying BAG source for details.

#[=======================================================================[.rst:
FindBAG
-------

Finds the BAG library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

  The baglib library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``BAG_FOUND``
  True if the system has the BAG library.
``BAG_VERSION``
  The version of the BAG library which was found.
``BAG_INCLUDE_DIRS``
  Include directories needed to use BAG.
``BAG_LIBRARIES``
  Libraries needed to link to BAG.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``BAG_INCLUDE_DIR``
  The directory containing ``bag_dataset.h``.
``BAG_LIBRARY``
  The path to the BAG library.

#]=======================================================================]

find_package(PkgConfig)
pkg_check_modules(PC_BAG QUIET BAG)

find_path(BAG_INCLUDE_DIR
  NAMES bag_dataset.h
  PATHS ${PC_BAG_INCLUDE_DIRS}
  PATH_SUFFIXES
    include
    include/bag
    include/BAG
    include/baglib
)
find_library(BAG_LIBRARY
  NAMES baglib
  PATHS ${PC_BAG_LIBRARY_DIRS}
)

set(BAG_VERSION ${PC_BAG_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BAG
  FOUND_VAR BAG_FOUND
  REQUIRED_VARS
  BAG_LIBRARY
  BAG_INCLUDE_DIR
  VERSION_VAR BAG_VERSION
)

if(BAG_FOUND)
  set(BAG_LIBRARIES ${BAG_LIBRARY})
  set(BAG_INCLUDE_DIRS ${BAG_INCLUDE_DIR})
  set(BAG_DEFINITIONS ${PC_BAG_CFLAGS_OTHER})
endif()

mark_as_advanced(
  BAG_INCLUDE_DIR
  BAG_LIBRARY
)
