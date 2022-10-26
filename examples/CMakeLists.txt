cmake_minimum_required(VERSION 3.15)
project(OPENNS_Examples LANGUAGES C CXX)

#-----------------------------------------------------------------------------
# Define Sources
#-----------------------------------------------------------------------------
set(examples
    bag_compound_layer
    bag_create
    bag_read
    bag_vr_create
    bag_vr_read
)

foreach(example ${examples})
    add_executable(${example} ${example}.cpp getopt.c getopt.h)

    target_compile_definitions(${example}
        PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:_USE_MATH_DEFINES>
            $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
            $<$<CXX_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_DEPRECATE>
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:BAG_BUILD_SHARED_LIBS>>:BAG_DLL>
    )

    target_compile_options(baglib
        PRIVATE
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.10>>:/permissive- /W4 /WX>
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.13>>:/experimental:external /external:W0 /external:templates->
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.13>>:/external:anglebrackets>
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19.14>>:/Zc:__cplusplus>
    )

    target_link_libraries(${example}
        PRIVATE
        baglib
    )

    set_target_properties(${example}
        PROPERTIES
            CMAKE_CXX_EXTENSIONS OFF
            FOLDER examples
            DEBUG_OUTPUT_NAME ${example}_d
            RELEASE_OUTPUT_NAME ${example}
    )

    # Install
    install(TARGETS ${example} RUNTIME DESTINATION bin)
endforeach()

