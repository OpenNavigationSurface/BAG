project(OpenNavSurf-BAG_Tests LANGUAGES CXX)

enable_testing()

# The debug build will have a 'd' postfix
set(CMAKE_DEBUG_POSTFIX "d")

set(TEST_SOURCE_FILES
    test_main.cpp
    test_bag_compound_layer.cpp
    test_bag_compounddatatype.cpp
    test_bag_dataset.cpp
    test_bag_descriptor.cpp
    test_bag_interleavedlegacylayer.cpp
    test_bag_interleavedlegacylayerdescriptor.cpp
    test_bag_metadata.cpp
    test_bag_record.cpp
    test_bag_simplelayer.cpp
    test_bag_simplelayerdescriptor.cpp
    test_bag_surfacecorrectionsdescriptor.cpp
    test_bag_surfacecorrections.cpp
    test_bag_trackinglist.cpp
    test_bag_valuetable.cpp
    test_utils.cpp
    test_utils.h
    test_bag_vrmetadata.cpp
    test_bag_vrmetadatadescriptor.cpp
    test_bag_vrnode.cpp
    test_bag_vrnodedescriptor.cpp
    test_bag_vrrefinements.cpp
    test_bag_vrrefinementsdescriptor.cpp
    test_bag_vrtrackinglist.cpp
)

source_group("Source Files" FILES ${TEST_SOURCE_FILES})

add_executable(bag_tests ${TEST_SOURCE_FILES})

set_target_properties(bag_tests
    PROPERTIES
        CXX_STANDARD 14
        CMAKE_CXX_STANDARD_REQUIRED ON
        CMAKE_CXX_EXTENSIONS OFF
        DEBUG_OUTPUT_NAME bag_tests_d
)

target_compile_definitions(bag_tests
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:_USE_MATH_DEFINES>
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_DEPRECATE>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:BAG_BUILD_SHARED_LIBS>>:BAG_DLL>
        CATCH_CONFIG_FAST_COMPILE
        CATCH_CONFIG_DISABLE_MATCHERS
        CATCH_CONFIG_ENABLE_TUPLE_STRINGMAKER
)

target_compile_options(bag_tests
    PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.10>>:/permissive- /W4 /WX>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.13>>:/experimental:external /external:W0 /external:templates->
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.13>>:/external:anglebrackets>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.14>>:/Zc:__cplusplus>
)

find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(Catch2 3 REQUIRED)

target_link_libraries(bag_tests
    PRIVATE
        baglib
        Catch2::Catch2
        HDF5::HDF5
)

include(CTest)
include(Catch)

if(CODE_COVERAGE)
    target_code_coverage(bag_tests AUTO ALL EXCLUDE tests/*)
endif()
