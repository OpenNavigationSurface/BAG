# Build Python bindings

# copied from BAG_HEADER_FILES.
# Other non-header files might need to be added,
# and some of these might not be needed?
set(BAG_SWIG_INPUT_FILES
    bag_c_types
    bag_uint8array
    bag_metadatatypes
    bag_types
    bag_metadata
    bag_metadata_import
    bag_metadata_export
    bag_exceptions
    bag_layerdescriptor
    bag_descriptor
    bag_interleavedlayerdescriptor
    bag_simplelayerdescriptor
    bag_surfacecorrectionsdescriptor
    bag_trackinglist
    bag_compounddatatype
    bag_valuetable
    bag_layer
    bag_simplelayer
    bag_interleavedlayer
    bag_surfacecorrections
    bag_compoundlayer
    bag_compoundlayerdescriptor
    bag_dataset
    bagpy

    ## not sure about these?
    #   #bag_xml_meta
    #   bag.hdfhelper
    #   bag_legacy_crs
    #   #bag_metadata_def
    #   #bag_opt_surfaces

    ## these should not be needed:
    #   bag_config
    #   bag_errors
    #   bag_fordec
    #   bag_version
)

# https://cmake.org/cmake/help/latest/module/FindPython3.html
#testing: http://www.swig.org/Doc4.0/Introduction.html#Introduction_build_system
FIND_PACKAGE(PythonLibs COMPONENTS Interpreter Development REQUIRED)
#this one doesn't find Python.h but above one does
#find_package(Python3 COMPONENTS Interpreter Development REQUIRED)  #TODO Is Python 2 OK?
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

# https://cmake.org/cmake/help/latest/module/FindSWIG.html
find_package(SWIG 4.0.1 REQUIRED COMPONENTS python)
# https://cmake.org/cmake/help/v3.12/module/UseSWIG.html
include(${SWIG_USE_FILE})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# The -importall directive tells SWIG to follow all #include statements as imports.
# This might be useful if you want to extract type definitions from system header files without generating any wrappers.
# http://www.swig.org/Doc4.0/Preprocessor.html#Preprocessor_nn3

set(CMAKE_SWIG_FLAGS "-Wall" "-v")
#set(CMAKE_SWIG_FLAGS "-debug-symtabs") #write out symbol tables when building

# foreach(ifile IN LISTS BAG_SWIG_INPUT_FILES)


    SET_SOURCE_FILES_PROPERTIES("bagpy.i" PROPERTIES
        SWIG_MODULE_NAME bagPy
        CPLUSPLUS ON
        COMPILE_FLAGS "--std=c++14"
        )

    swig_add_library(bagPy TYPE SHARED LANGUAGE python
        OUTFILE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../swig_wrapper"
        OUTPUT_DIR "${CMAKE_HOME_DIRECTORY}/python"  # change the dir of output .py files. Compiled pyd files get moved by later custom_command
        SOURCES "bagpy.i" #${CMAKE_CURRENT_SOURCE_DIR}/${ifile}.h
        )

    target_include_directories(bagPy
        PUBLIC 
            ${HDF5_INCLUDE_DIRS}
            ${PYTHON_INCLUDE_DIRS}
    )

    swig_link_libraries(bagPy
        baglib
        ${PYTHON_LIBRARIES} 
        ${HDF5_hdf5_LIBRARY}
        ${HDF5_hdf5_cpp_LIBRARY}
        LibXml2::LibXml2)

    # Install
    # Get the python site packages directory by invoking python
    #execute_process(COMMAND ${Python3_EXECUTABLE} -c "import site; print(site.getsitepackages()[0])" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
    #message("PYTHON_SITE_PACKAGES = ${PYTHON_SITE_PACKAGES}")
    #install(TARGETS _bag DESTINATION ${PYTHON_SITE_PACKAGES})
    #install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bag.py DESTINATION ${PYTHON_SITE_PACKAGES})
        
# endforeach()

# copy the generated pyd file to test dir
add_custom_command(TARGET bagPy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bagPy> "${CMAKE_HOME_DIRECTORY}/python"
)
