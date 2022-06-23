# The debug build will have a 'd' postfix
set(CMAKE_DEBUG_POSTFIX "d")

set(BAG_SOURCE_FILES
    bag.cpp
    bag_attributeinfo.cpp
    bag_compoundlayer.cpp
    bag_compoundlayerdescriptor.cpp
    bag_dataset.cpp
    bag_deleteh5dataset.cpp
    bag_descriptor.cpp
    bag_hdfhelper.cpp
    bag_interleavedlegacylayer.cpp
    bag_interleavedlegacylayerdescriptor.cpp
    bag_layer.cpp
    bag_layerdescriptor.cpp
    bag_legacy_crs.cpp
    bag_metadata.cpp
    bag_metadata_export.cpp
    bag_metadata_import.cpp
    bag_metadataprofiles.cpp
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
    bag_vrrefinements.cpp
    bag_vrrefinementsdescriptor.cpp
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
    bag_deleteh5dataset.h
    bag_descriptor.h
    bag_errors.h
    bag_exceptions.h
    bag_fordec.h
    bag_hdfhelper.h
    bag_interleavedlegacylayer.h
    bag_interleavedlegacylayerdescriptor.h
    bag_layer.h
    bag_layerdescriptor.h
    bag_layeritems.h
    bag_legacy_crs.h
    bag_metadata.h
    bag_metadata_export.h
    bag_metadata_import.h
    bag_metadataprofiles.h
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
    bag_vrrefinements.h
    bag_vrrefinementsdescriptor.h
    bag_vrtrackinglist.h
    bag_types.h
    bag_uint8array.h
    bag_valuetable.h
    bag_version.h
)
source_group("Header Files" FILES ${BAG_HEADER_FILES} ${BAG_PRIVATE_HEADER_FILES})

# Allow for inserting BAG as a nested library (not tested)
if(BAG_BUILD_SHARED_LIBS)
    add_library(baglib SHARED
        ${BAG_SOURCE_FILES} ${BAG_HEADER_FILES} ${BAG_PRIVATE_HEADER_FILES}
    )
else()
    add_library(baglib STATIC
        ${BAG_SOURCE_FILES} ${BAG_HEADER_FILES} ${BAG_PRIVATE_HEADER_FILES}
    )
endif()

set_target_properties(baglib
    PROPERTIES
        CXX_STANDARD 14
        CMAKE_CXX_STANDARD_REQUIRED ON
        CMAKE_CXX_EXTENSIONS OFF
)

target_compile_definitions(baglib
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:_USE_MATH_DEFINES>
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_DEPRECATE>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:BAG_BUILD_SHARED_LIBS>>:BAG_DLL>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:BAG_BUILD_SHARED_LIBS>>:BAG_EXPORTS>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:BAG_BUILD_SHARED_LIBS>>:H5_BUILT_AS_DYNAMIC_LIB>
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

# Windows is combining debug and release libraries unless we do this.
# Initial attempt was:
#   $<IF:$<CXX_COMPILER_ID:MSVC>,"${HDF5_hdf5_LIBRARY} ${HDF5_hdf5_cpp_LIBRARY}","${HDF5_LIBRARIES}">
# But it will not parse correctly.
if(MSVC)
    target_link_libraries(baglib
        PUBLIC
            ${HDF5_hdf5_LIBRARY}
            ${HDF5_hdf5_cpp_LIBRARY}
        PRIVATE
            LibXml2::LibXml2
            HDF5::HDF5
    )

    if(NOT BAG_CI)
        # Copy DLLs into the runtime output directory so running C++ tests is smooth.
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

        list(LENGTH FOUND_DLLS NUM_FOUND_DLLS)
        if(${NUM_FOUND_DLLS} GREATER 0)
            add_custom_command(TARGET baglib POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FOUND_DLLS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            )
        endif()
    endif()
else()
    target_link_libraries(baglib
        PUBLIC
            ${HDF5_LIBRARIES}
        PRIVATE
            LibXml2::LibXml2
            HDF5::HDF5
    )
endif()

if(BAG_BUILD_SWIG)
    # Build SWIG bindings
    add_subdirectory(swig)
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

