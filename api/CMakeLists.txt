
# The debug build will have a 'd' postfix
set(CMAKE_DEBUG_POSTFIX "d")

#needed when CMakeSettings.json is not available
#list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/../../third_party/build;${CMAKE_SOURCE_DIR}/../../third_party/build/lib" )

set(BAG_SOURCE_FILES
    bag.cpp
    bag_attributeinfo.cpp
    bag_compoundlayer.cpp
    bag_compoundlayerdescriptor.cpp
    bag_dataset.cpp
    bag_descriptor.cpp
    bag_hdfhelper.cpp
    bag_interleavedlayer.cpp
    bag_interleavedlayerdescriptor.cpp
    bag_layer.cpp
    bag_layerdescriptor.cpp
    bag_legacy_crs.cpp
    bag_metadata.cpp
    bag_metadata_export.cpp
    bag_metadata_import.cpp
    bag_metadatatypes.cpp
    bag_simplelayer.cpp
    bag_simplelayerdescriptor.cpp
    bag_surfacecorrections.cpp
    bag_surfacecorrectionsdescriptor.cpp
    bag_trackinglist.cpp
    bag_valuetable.cpp
    bag_vrmetadata.cpp
    bag_vrmetadatadescriptor.cpp
    bag_vrnode.cpp
    bag_vrnodedescriptor.cpp
    bag_vrrefinement.cpp
    bag_vrrefinementdescriptor.cpp
    bag_vrtrackinglist.cpp
)
source_group("Source Files" FILES ${BAG_SOURCE_FILES})

set(BAG_PRIVATE_HEADER_FILES
    bag_private.h
)

set(BAG_HEADER_FILES
    bag.h
    bag_attributeinfo.h
    bag_c_types.h
    bag_compounddatatype.h
    bag_compoundlayer.h
    bag_compoundlayerdescriptor.h
    bag_config.h
    bag_dataset.h
    bag_descriptor.h
    bag_errors.h
    bag_exceptions.h
    bag_fordec.h
    bag_hdfhelper.h
    bag_interleavedlayer.h
    bag_interleavedlayerdescriptor.h
    bag_layer.h
    bag_layerdescriptor.h
    bag_legacy_crs.h
    bag_metadata.h
    bag_metadata_export.h
    bag_metadata_import.h
    bag_metadatatypes.h
    bag_simplelayer.h
    bag_simplelayerdescriptor.h
    bag_surfacecorrections.h
    bag_surfacecorrectionsdescriptor.h
    bag_trackinglist.h
    bag_vrmetadata.h
    bag_vrmetadatadescriptor.h
    bag_vrnode.h
    bag_vrnodedescriptor.h
    bag_vrrefinement.h
    bag_vrrefinementdescriptor.h
    bag_vrtrackinglist.h
    bag_types.h
    bag_valuetable.h
    bag_version.h
)
source_group("Header Files" FILES ${BAG_HEADER_FILES} ${BAG_PRIVATE_HEADER_FILES})

add_library(baglib
    ${BAG_SOURCE_FILES} ${BAG_HEADER_FILES} ${BAG_PRIVATE_HEADER_FILES}
)

set_target_properties(baglib
    PROPERTIES
        CXX_STANDARD 14
        CMAKE_CXX_STANDARD_REQUIRED ON
        CMAKE_CXX_EXTENSIONS OFF
)

#TODO if shared, add -fpic for non-MSVC compiler

target_compile_definitions(baglib
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:_USE_MATH_DEFINES>
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_DEPRECATE>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:BUILD_SHARED_LIBS>>:BAG_DLL>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:BUILD_SHARED_LIBS>>:BAG_EXPORTS>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:BUILD_SHARED_LIBS>>:H5_BUILT_AS_DYNAMIC_LIB>
)

# MSVC compiler versions.
# From https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-by-compiler-version?view=vs-2019
# 19.00     2015
# 19.10     2017
# 19.20     2019
target_compile_options(baglib
    PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.10>>:/permissive- /W4 /WX>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.13>>:/experimental:external /external:W0 /external:templates->
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.13>>:/external:anglebrackets>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.14>>:/Zc:__cplusplus>
)

find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(LibXml2 MODULE REQUIRED)

target_include_directories(baglib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE SYSTEM
        ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(baglib
    PRIVATE
        ${HDF5_hdf5_LIBRARY}
        ${HDF5_hdf5_cpp_LIBRARY}
        LibXml2::LibXml2
)


# Build Python bindings
if(BUILD_PYTHON)
    # copied from BAG_HEADER_FILES.
    # Other non-header files might need to be added,
    # and some of these might not be needed?
    set(BAG_SWIG_INPUT_FILES
        bag_c_types
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


        ## temp disabled to avoid warnings:
        # bag_trackinglist      #needs overloaded operator[] and push_back(), etc to be redefined
        # bag_compounddatatype  #need to handle overloaded operator=
        # bag_valuetable        #compiles, just needs bag_compounddatatype enabled

        # bag_layer     # need to add support for std::unique_ptr<uint8_t>


        ## these need layer:
        #   bag_compoundlayer
        #   bag_compoundlayerdescriptor
        #   bag_interleavedlayer
        #   bag_simplelayer
        #   bag_surfacecorrections

        #  bag_dataset         #needs all of the above?


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
    find_package(SWIG 4.0.1 REQUIRED)
    # https://cmake.org/cmake/help/v3.12/module/UseSWIG.html
    include(${SWIG_USE_FILE})

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

    # The -importall directive tells SWIG to follow all #include statements as imports.
    # This might be useful if you want to extract type definitions from system header files without generating any wrappers.
    # http://www.swig.org/Doc4.0/Preprocessor.html#Preprocessor_nn3

    set(CMAKE_SWIG_FLAGS "-Wall" "-v")
    #set(CMAKE_SWIG_FLAGS "-debug-symtabs") #write out symbol tables when building

    foreach(ifile IN LISTS BAG_SWIG_INPUT_FILES)

        #ATTEMPT1
        ###set_source_files_properties(file.i PROPERTIES SWIG_FLAGS "-threads")
        ###set_source_files_properties(bag_metadata.i PROPERTIES SWIG_FLAGS "-v -c++ -cppext cpp")


        SET_SOURCE_FILES_PROPERTIES("interface/${ifile}.i" PROPERTIES
            CPLUSPLUS ON
            COMPILE_FLAGS "--std=c++14"
            )

        swig_add_library(${ifile} TYPE SHARED LANGUAGE python
            OUTFILE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/swig_wrapper"
            #OUTPUT_DIR "${CMAKE_BINARY_DIR}/swig_python"  # change the dir of output .py files, but .pyd files don't move so this doesn't work
            SOURCES "interface/${ifile}.i"
            )
        SWIG_LINK_LIBRARIES(${ifile} baglib ${PYTHON_LIBRARIES})

        #END_ATTEMPT1

        ###OLD:


        #ATTEMPT2
        #    target_include_directories(pyBag PRIVATE ${Python3_INCLUDE_DIRS})
        #    set_property(TARGET pyBag PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON)
        #    target_link_libraries(pyBag PRIVATE bag ${Python3_LIBRARIES})
        #END_ATTEMPT2

        # Install
        # Get the python site packages directory by invoking python
        #execute_process(COMMAND ${Python3_EXECUTABLE} -c "import site; print(site.getsitepackages()[0])" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
        #message("PYTHON_SITE_PACKAGES = ${PYTHON_SITE_PACKAGES}")
        #install(TARGETS _bag DESTINATION ${PYTHON_SITE_PACKAGES})
        #install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bag.py DESTINATION ${PYTHON_SITE_PACKAGES})
    endforeach()
endif()


# Install
install(FILES
    ${BAG_HEADER_FILES} DESTINATION include
)

install(TARGETS baglib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
)

install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/configdata/ DESTINATION configdata
)

