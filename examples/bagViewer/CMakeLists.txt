cmake_minimum_required(VERSION 2.8.11)

project(helloQT)

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
target_link_libraries(bagViewer ${QT_LIBRARIES} ${OPENGL_gl_LIBRARY})
