# Build Python bindings

# https://cmake.org/cmake/help/latest/module/FindPython3.html
#testing: http://www.swig.org/Doc4.0/Introduction.html#Introduction_build_system
FIND_PACKAGE(PythonLibs COMPONENTS Interpreter Development REQUIRED)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

# https://cmake.org/cmake/help/latest/module/FindSWIG.html
find_package(SWIG 4.0.1 REQUIRED COMPONENTS python)
# https://cmake.org/cmake/help/v3.12/module/UseSWIG.html
include(${SWIG_USE_FILE})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_SWIG_FLAGS "-Wall" "-v") #turn on all warnings and verbose output

SET_SOURCE_FILES_PROPERTIES("test_utils.i" PROPERTIES
    SWIG_MODULE_NAME bagTestPy
    CPLUSPLUS ON
    COMPILE_FLAGS "--std=c++14"
)

swig_add_library(bagTestPy TYPE SHARED LANGUAGE python
    OUTFILE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../swig_wrapper" # change the dir of generated wrapper file
    OUTPUT_DIR "${CMAKE_HOME_DIRECTORY}/python"  # change the dir of output .py file
    SOURCES "test_utils.i"
)

target_include_directories(bagTestPy
    PUBLIC ${PYTHON_INCLUDE_DIRS}
)

swig_link_libraries(bagTestPy
    ${PYTHON_LIBRARIES}
)

# copy the generated .pyd file to test dir
add_custom_command(TARGET bagTestPy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bagTestPy> "${CMAKE_HOME_DIRECTORY}/python"
)
