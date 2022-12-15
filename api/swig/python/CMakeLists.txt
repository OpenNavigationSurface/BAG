find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
include_directories(../..)

set_property(GLOBAL PROPERTY BAGPY_BINARY_DIR_PROP ${CMAKE_CURRENT_BINARY_DIR})
set(BAGPY_MODULE_NAME bagPy.py)

set(CMAKE_SWIG_FLAGS "-Wextra" "-v" "-py3")

set_source_files_properties("bagpy.i" PROPERTIES
    SWIG_MODULE_NAME bagPy
    CPLUSPLUS ON
)

# Proper Python exception generation results in larger code, which MSVC can't handle by default 
if (MSVC)
    add_compile_options(/bigobj)
endif()

swig_add_library(bagPy TYPE SHARED LANGUAGE python
    SOURCES "bagpy.i"
)

swig_link_libraries(bagPy
    baglib
    Python3::Python
)

if(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
    if(${CMAKE_BUILD_TYPE} STREQUAL Release OR ${CMAKE_BUILD_TYPE} STREQUAL MinSizeRel)
        set(BAGPY_LIB _bagPy.so)
    else()
        set(BAGPY_LIB _bagPyd.so)
    endif()
    # Somehow CPython on macOS looks for native code to end in .so instead of .dylib so copy the
    # compiled extension to a file ending in .so...
    add_custom_command(TARGET bagPy POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:bagPy> "${BAGPY_LIB}")
elseif(${CMAKE_SYSTEM_NAME} MATCHES Linux)
    if(${CMAKE_BUILD_TYPE} STREQUAL Release OR ${CMAKE_BUILD_TYPE} STREQUAL MinSizeRel)
        set(BAGPY_LIB _bagPy.so)
    else()
        set(BAGPY_LIB _bagPyd.so)
    endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    # TODO: Adjust this for debug builds on Windows as above
    set(BAGPY_LIB _bagPy.pyd)
endif()

# Files to install with Python
set(PYTHON_INSTALL_FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${BAGPY_MODULE_NAME}
        ${CMAKE_CURRENT_BINARY_DIR}/${BAGPY_LIB})

# Configure setup.py and copy to output directory
set(SETUP_PY_IN ${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in)
set(SETUP_PY_OUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
configure_file(${SETUP_PY_IN} ${SETUP_PY_OUT})

# Install target to call setup.py
add_custom_target(install-python
        DEPENDS _bagPy
        COMMAND python ${SETUP_PY_OUT} install)
