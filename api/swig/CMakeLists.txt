# Set up swig
find_package(SWIG 4 REQUIRED)
include(${SWIG_USE_FILE})

# Add subdirectories for each language if desired
if(BAG_BUILD_PYTHON)
    add_subdirectory(python)
endif()
