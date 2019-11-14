
# The debug build will have a 'd' postfix
set(CMAKE_DEBUG_POSTFIX "d")

set(BAG_SOURCE_FILES
    bag.cpp
    bag_compoundlayer.cpp
    bag_dataset.cpp
    bag_descriptor.cpp
    #bag_hdf.c
    bag_interleavedlayer.cpp
    bag_interleavedlayerdescriptor.cpp
    bag_layer.cpp
    bag_layerdescriptor.cpp
    bag_legacy_crs.cpp
    bag_metadata.cpp
    bag_metadata_export.cpp
    bag_metadata_import.cpp
    bag_metadatatypes.cpp
    #bag_opt_group.c                TODO
    #bag_opt_surfaces.c             TODO
    #bag_surface_correct.c
    #bag_surfaces.c
    #bag_tracking_list.c
    bag_simplelayer.cpp
    bag_simplelayerdescriptor.cpp
    bag_surfacecorrections.cpp
    bag_surfacecorrectionsdescriptor.cpp
    bag_trackinglist.cpp
)
source_group("Source Files" FILES ${BAG_SOURCE_FILES})

set(BAG_PRIVATE_HEADER_FILES
    bag_private.h
)

set(BAG_HEADER_FILES
    bag.h
    bag_c_types.h
    bag_compounddatatype.h
    bag_compoundlayer.h
    bag_config.h
    bag_dataset.h
    bag_descriptor.h
    bag_errors.h
    bag_exceptions.h
    bag_fordec.h
    bag_interleavedlayer.h
    bag_interleavedlayerdescriptor.h
    bag_layer.h
    bag_layerdescriptor.h
    bag_legacy_crs.h
    bag_metadata.h
    #bag_metadata_def.h
    bag_metadata_export.h
    bag_metadata_import.h
    bag_metadatatypes.h
    #bag_opt_surfaces.h
    bag_simplelayer.h
    bag_simplelayerdescriptor.h
    bag_surfacecorrections.h
    bag_surfacecorrectionsdescriptor.h
    bag_trackinglist.h
    bag_types.h
    bag_version.h
    #bag_xml_meta.h
)
source_group("Header Files" FILES ${BAG_HEADER_FILES} ${BAG_PRIVATE_HEADER_FILES})

add_library(bag
    ${BAG_SOURCE_FILES} ${BAG_HEADER_FILES} ${BAG_PRIVATE_HEADER_FILES}
)

set_target_properties(bag
    PROPERTIES
        CXX_STANDARD 14
        CMAKE_CXX_STANDARD_REQUIRED ON
        CMAKE_CXX_EXTENSIONS OFF
)

#TODO if shared, add -fpic for non-MSVC compiler

target_compile_definitions(bag
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
target_compile_options(bag
    PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.10>>:/permissive- /W4 /WX>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.13>>:/experimental:external /external:W0 /external:templates->
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.13>>:/external:anglebrackets>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.14>>:/Zc:__cplusplus>
)

find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(LibXml2 MODULE REQUIRED)

target_include_directories(bag
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE SYSTEM
        ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(bag
    PRIVATE
        ${HDF5_hdf5_LIBRARY}
        ${HDF5_hdf5_cpp_LIBRARY}
        LibXml2::LibXml2
)

# Install
install(FILES
    ${BAG_HEADER_FILES} DESTINATION include
)

install(TARGETS bag
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
)

install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/configdata/ DESTINATION configdata
)

