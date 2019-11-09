cmake_minimum_required(VERSION 3.0...3.15)
project(OpenNS_Tests LANGUAGES CXX)

enable_testing()

# The debug build will have a 'd' postfix
set(CMAKE_DEBUG_POSTFIX "d")

set(TEST_SOURCE_FILES
    test_main.cpp
    test_bag_dataset.cpp
    test_bag_descriptor.cpp
    test_bag_interleavedlayer.cpp
    test_bag_interleavedlayerdescriptor.cpp
    test_bag_metadata.cpp
    test_bag_simplelayer.cpp
    test_bag_simplelayerdescriptor.cpp
    test_bag_surfacecorrections.cpp
    test_bag_trackinglist.cpp
    test_utils.cpp
    test_utils.h
)

add_executable(bag_tests ${TEST_SOURCE_FILES})

set_target_properties(bag_tests
    PROPERTIES
        CXX_STANDARD 14
        CMAKE_CXX_STANDARD_REQUIRED ON
        CMAKE_CXX_EXTENSIONS OFF
)

target_compile_definitions(bag_tests
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:_USE_MATH_DEFINES>
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_DEPRECATE>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:BUILD_SHARED_LIBS>>:BAG_DLL>
        CATCH_CONFIG_FAST_COMPILE
        CATCH_CONFIG_DISABLE_MATCHERS
)
        #GTEST_LINKED_AS_SHARED_LIBRARY=1

target_compile_options(bag_tests
    PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/permissive- /W4 /WX /Zc:__cplusplus>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/experimental:external /external:W0 /external:templates->
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/external:I ${HDF5_INCLUDE_DIRS} >
)

find_package(HDF5 REQUIRED)

# Catch2
find_package(Catch2 REQUIRED)

target_link_libraries(bag_tests
    PRIVATE
        bag
        Catch2::Catch2
        debug ${HDF5_hdf5_LIBRARY_DEBUG} ${HDF5_hdf5_cpp_LIBRARY_DEBUG}
        optimized ${HDF5_hdf5_LIBRARY_RELEASE} ${HDF5_hdf5_cpp_LIBRARY_RELEASE}
)

include(CTest)
include(Catch)
catch_discover_tests(bag_tests EXTRA_ARGS --samples-path "D:/caris_work/BAG/BAG2/examples/sample-data")

# https://cmake.org/cmake/help/v3.15/module/GoogleTest.html
# GoogleTest
#find_package(GTest REQUIRED)

#gtest_discover_tests(bag_tests) # EXTRA_ARGS --samples-path "D:/caris_work/BAG/BAG2/examples/sample-data"
