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
    add_custom_command(TARGET bagPy POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:bagPy> "_bagPy.so")
    set(BAGPY_LIB _bagPy.so)
elseif(${CMAKE_SYSTEM_NAME} MATCHES Linux)
    set(BAGPY_LIB _bagPy.so)
elseif(${CMAKE_SYSTEM_NAME} MATCHES Windows)
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
