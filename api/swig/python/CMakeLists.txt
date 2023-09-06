# Generate standard target names.
cmake_policy(SET CMP0078 NEW)
# Honor SWIG_MODULE_NAME via -module flag.
cmake_policy(SET CMP0086 NEW)

if(Python_LOOKUP_VERSION)
    set(Python_FIND_STRATEGY VERSION)
    find_package(Python3 ${Python_LOOKUP_VERSION} COMPONENTS Interpreter Development.Module)
else()
    set(Python_FIND_STRATEGY LOCATION)
    find_package(Python3 COMPONENTS Interpreter Development.Module)
endif()

# Handle where to install the resulting Python package
if (CALL_FROM_SETUP_PY)
    # The CMakeExtension will set CMAKE_INSTALL_PREFIX to the root
    # of the resulting wheel archive
    set(BAGPY_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
else()
    # The Python package is installed directly in the folder of the
    # detected interpreter (system, user, or virtualenv)
    set(BAGPY_INSTALL_PREFIX ${Python3_SITELIB})
endif()

if(DEFINED Python3_INCLUDE_DIRS)
    include_directories("${CMAKE_SOURCE_DIR}/api" "${Python3_INCLUDE_DIRS}")
else()
    include_directories("${CMAKE_SOURCE_DIR}/api")
endif()

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
set_target_properties(bagPy
    PROPERTIES DEBUG_POSTFIX ""
)

target_link_libraries(bagPy
    PRIVATE baglib
    PRIVATE Python3::Module
)

if(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
    set(BAGPY_LIB _bagPy.so)
    # Somehow CPython on macOS looks for native code to end in .so instead of .dylib so copy the
    # compiled extension to a file ending in .so...
    add_custom_command(TARGET bagPy POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:bagPy> "${BAGPY_LIB}")
elseif(${CMAKE_SYSTEM_NAME} MATCHES Linux)
    set(BAGPY_LIB _bagPy.so)
elseif(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    set(BAGPY_LIB _bagPy.pyd)
endif()

# Install the wrapper.
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BAGPY_LIB} DESTINATION ${BAGPY_INSTALL_PREFIX})

# Install swig-generated Python file (we rename it to __init__.py as it will
# ultimately end up in a directory called bagPy in site-packages, which will
# serve as the module directory.
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/bagPy.py"
        DESTINATION ${BAGPY_INSTALL_PREFIX}
        RENAME __init__.py
        COMPONENT bagPy)
