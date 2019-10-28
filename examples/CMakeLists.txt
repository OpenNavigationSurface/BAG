cmake_minimum_required(VERSION 3.0...3.15)
project(OpenNS_Examples LANGUAGES C CXX)

#-----------------------------------------------------------------------------
# Define Sources
#-----------------------------------------------------------------------------
set(examples
#    bag_create
#    bag_vr_create
#    bag_gencert
    bag_read
#    bag_vr_read
#    bag_signcert
#    bag_signfile
#    bag_verifycert
#    bag_verifyfile
)

foreach(example ${examples})
    add_executable(${example} ${example}.cpp)

    target_compile_definitions(${example}
        PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:_USE_MATH_DEFINES>
            $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
            $<$<CXX_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_DEPRECATE>
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:BUILD_SHARED_LIBS>>:BAG_DLL>
    )

    target_compile_options(bag
        PRIVATE
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/permissive- /W4 /WX /Zc:__cplusplus>
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/experimental:external /external:W0 /external:templates->
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/external:I ${HDF5_INCLUDE_DIRS} >
    )

    #Link with the bag library.
    target_link_libraries(${example} bag)

    #Add the '_debug' postfix to the samples.
    set_target_properties(${example}
        PROPERTIES
            CXX_STANDARD 14
            CMAKE_CXX_STANDARD_REQUIRED ON
            CMAKE_CXX_EXTENSIONS OFF
            FOLDER examples
            DEBUG_OUTPUT_NAME ${example}_debug
            RELEASE_OUTPUT_NAME ${example}
    )

#    if(NOT MSVC)
#        set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
#    endif()


    install(TARGETS ${example} RUNTIME DESTINATION bin)
endforeach()

