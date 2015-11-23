cmake_minimum_required(VERSION 2.8.11)

# Eliminate a CMake warning for policy CMP0043.
# This policy was CMake version 3.0.2 warns when the policy 
# is not set and uses OLD behavior. 
# We Use the cmake_policy command to set it to OLD explicitly.
if( POLICY CMP0043 )
	cmake_policy( SET CMP0043 OLD )
endif()

project(bagViewer)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5 COMPONENTS Widgets OpenGL)

find_package(OpenGL)

add_custom_command(
  OUTPUT ui_BagViewer.h
  COMMAND uic 
  ARGS    ${PROJECT_SOURCE_DIR}/BagViewer.ui -o ${PROJECT_BINARY_DIR}/ui_BagViewer.h
  DEPENDS BagViewer.ui 
)

set_property(SOURCE BagViewer.cpp APPEND PROPERTY OBJECT_DEPENDS ${PROJECT_BINARY_DIR}/ui_BagViewer.h)

add_executable(bagViewer main.cpp GLWindow.cpp BagViewer.cpp BagGL.cpp)
qt5_use_modules(bagViewer Widgets OpenGL)
target_link_libraries(bagViewer ${QT_LIBRARIES} ${OPENGL_gl_LIBRARY} bag)
