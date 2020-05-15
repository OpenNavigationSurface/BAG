# Build Python bindings

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(SWIG 4.0.1 REQUIRED COMPONENTS python)

include(${SWIG_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Turn on all warnings and verbose output
set(CMAKE_SWIG_FLAGS "-Wall" "-Wextra" "-v" "-builtin")

set_source_files_properties("bagpy.i" PROPERTIES
    SWIG_MODULE_NAME bagPy
    CPLUSPLUS ON
    COMPILE_FLAGS "--std=c++14"
)

swig_add_library(bagPy TYPE SHARED LANGUAGE python
    SOURCES "bagpy.i"
)

swig_link_libraries(bagPy
    baglib
    Python3::Python
)

# Copy the generated Python shared library file to test directory.
add_custom_command(TARGET bagPy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:bagPy> "${CMAKE_HOME_DIRECTORY}/python"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/api/swig_i/bagPy.py "${CMAKE_HOME_DIRECTORY}/python"
)

if(MSVC)
    # Copy DLLs into the python directory so running Python tests is smooth.
    # Find Zlib to copy the DLLs.
    find_package(ZLIB REQUIRED)

    # Find all the runtime dependencies (DLLs) using CMAKE_PREFIX_PATH.

    set(THIRD_PARTY_LIBS
        ${HDF5_hdf5_LIBRARY_DEBUG}
        ${HDF5_hdf5_LIBRARY_RELEASE}
        ${LIBXML2_LIBRARIES}
        ${ZLIB_LIBRARY_DEBUG}
        ${ZLIB_LIBRARY_RELEASE}
    )

    # Trim the path down to just the file name.
    foreach(LIB ${THIRD_PARTY_LIBS})
        get_filename_component(FILENAME ${LIB} NAME_WLE)
        list(APPEND THIRD_PARTY_DLLS ${FILENAME}*.dll)
    endforeach()

    # Search through CMAKE_PREFIX_PATH for each DLL glob.
    foreach(CPPATH ${CMAKE_PREFIX_PATH})
        foreach(DLL ${THIRD_PARTY_DLLS})
            set(PATH_DLL ${CPPATH}/**/${DLL})

            file(GLOB FOUND_DLL LIST_DIRECTORIES false ${PATH_DLL})

            list(LENGTH FOUND_DLL NUM_DLLS_FOUND)
            if(${NUM_DLLS_FOUND} GREATER 0)
                list(APPEND FOUND_DLLS ${FOUND_DLL})
            endif()
        endforeach()
    endforeach()

    add_custom_command(TARGET bagPy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FOUND_DLLS} "${CMAKE_HOME_DIRECTORY}/python"
    )
endif()

