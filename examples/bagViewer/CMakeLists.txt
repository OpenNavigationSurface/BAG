cmake_minimum_required(VERSION 3.5.1)

project(bagViewer)

# The debug build will have a 'd' postfix
SET(CMAKE_DEBUG_POSTFIX  "d")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Widgets OpenGL)

if (Qt5Widgets_FOUND)
    if (Qt5Widgets_VERSION VERSION_LESS 5.4.0)
        message(FATAL_ERROR "Minimum supported Qt5 version is 5.4!")
    endif()
else()
    message(SEND_ERROR "The Qt5Widgets library could not be found!")
endif(Qt5Widgets_FOUND)

find_package(OpenGL)

set_property(SOURCE BagViewer.cpp APPEND PROPERTY OBJECT_DEPENDS ${PROJECT_BINARY_DIR}/ui_BagViewer.h)

set (SHADERS vertex.glsl tes.glsl geometry.glsl fragment.glsl)

set ( RESOURCES
 resources.qrc
)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=c++11)
endif(CMAKE_COMPILER_IS_GNUCXX)

set( HDR BagViewer.h BagGL.h)
set( SRC main.cpp BagViewer.cpp BagGL.cpp BagIO.cpp)
if( MSVC )
	set( SRC ${SRC} BagViewer.rc )
endif()
add_executable(bagViewer ${HDR} ${SRC} ${RESOURCES})

qt5_use_modules(bagViewer Widgets OpenGL)
target_link_libraries(bagViewer ${OPENGL_gl_LIBRARY} bag ${QT_LIBRARIES})

INSTALL(TARGETS bagViewer RUNTIME DESTINATION bin)
