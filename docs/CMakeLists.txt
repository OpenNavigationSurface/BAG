# Adapted from: https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/
find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)
#include(${PROJECT_SOURCE_DIR}/api/CMakeLists.txt)
if(BAG_BUILD_PYTHON)
#    include(${PROJECT_SOURCE_DIR}/swig/python/CMakeLists.txt)
endif()

# Configure Doxygen to build XML docs for C++ API
set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/api)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

# Replace variables inside @@ with the current values
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

# Doxygen won't create this for us
file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

# Only regenerate Doxygen when the Doxyfile or public headers change
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
        DEPENDS ${BAG_HEADER_FILES}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
        COMMENT "Generating C++ API docs"
        VERBATIM)

# Nice named target so we can run the job easily
add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

# Configure Sphinx to generate overall documentation (How-to, FSD) as well as
# docs for the Python API
set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/sphinx/source)
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

# Only regenerate Sphinx when:
# - baglib has been updated (if BAG_BUILD_PYTHON is true)
# - bagPy has been updated (if BAG_BUILD_PYTHON is true)
# - Doxygen has rerun
# - Our doc files have been updated
# - The Sphinx config has been updated
if(BAG_BUILD_PYTHON)
    get_property(BAGPY_BINARY_DIR GLOBAL PROPERTY BAGPY_BINARY_DIR_PROP)
    get_property(BAGLIB_TARGET GLOBAL PROPERTY BAGLIB_TARGET_PROP)
    add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
            COMMAND
                # Copy baglib into build destination of SWIG Python bindings so that bagPy can be imported
                # by Sphinx to autogen docs.
                ${CMAKE_COMMAND} -E copy ${BAGLIB_TARGET} ${CMAKE_BINARY_DIR}/api/swig/python
            COMMAND
                # Run Sphinx with PYTHONPATH set to bagPy build dir so that bagPy can be imported to
                # autogen docs.
                ${CMAKE_COMMAND} -E env PYTHONPATH=${BAGPY_BINARY_DIR} ${SPHINX_EXECUTABLE} -b html
            # Tell Breathe where to find the Doxygen output
            -Dbreathe_projects.baglib=${DOXYGEN_OUTPUT_DIR}/xml
            ${SPHINX_SOURCE} ${SPHINX_BUILD}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS
            baglib # Don't try to generate docs until baglib and bagPy
            bagPy  # are available because Sphinx needs to import bagPy to autogen docs.
            ${SPHINX_SOURCE}/index.rst
            ${DOXYGEN_INDEX_FILE}
            MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
            COMMENT "Generating documentation with Sphinx")
elseif()
    add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
            COMMAND
            ${SPHINX_EXECUTABLE} -b html
            # Tell Breathe where to find the Doxygen output
            -Dbreathe_projects.baglib=${DOXYGEN_OUTPUT_DIR}/xml
            ${SPHINX_SOURCE} ${SPHINX_BUILD}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS
            ${SPHINX_SOURCE}/index.rst
            ${DOXYGEN_INDEX_FILE}
            MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
            COMMENT "Generating documentation with Sphinx")
endif()


# Nice named target so we can run the job easily
add_custom_target(Sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})
