# Build Python bindings

# https://cmake.org/cmake/help/latest/module/FindPython3.html
#testing: http://www.swig.org/Doc4.0/Introduction.html#Introduction_build_system
FIND_PACKAGE(Python3 COMPONENTS Interpreter Development REQUIRED)

# https://cmake.org/cmake/help/latest/module/FindSWIG.html
find_package(SWIG 4.0.1 REQUIRED COMPONENTS python)
# https://cmake.org/cmake/help/v3.12/module/UseSWIG.html
include(${SWIG_USE_FILE})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_SWIG_FLAGS "-Wall" "-v") #turn on all warnings and verbose output
#set(CMAKE_SWIG_FLAGS "-debug-symtabs") #write out symbol tables when building

SET_SOURCE_FILES_PROPERTIES("bagpy.i" PROPERTIES
    SWIG_MODULE_NAME bagPy
    CPLUSPLUS ON
    COMPILE_FLAGS "--std=c++14"
    )

swig_add_library(bagPy TYPE SHARED LANGUAGE python
    OUTFILE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../swig_wrapper" # change the dir of generated wrapper file
    OUTPUT_DIR "${CMAKE_HOME_DIRECTORY}/python"  # change the dir of output .py file
    SOURCES "bagpy.i"
    )

target_include_directories(bagPy
    PUBLIC 
        ${HDF5_INCLUDE_DIRS}
        ${PYTHON_INCLUDE_DIRS}
)

swig_link_libraries(bagPy
    baglib
    Python3::Python
)

# copy the generated .pyd file to test dir
add_custom_command(TARGET bagPy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bagPy> "${CMAKE_HOME_DIRECTORY}/python"
)
