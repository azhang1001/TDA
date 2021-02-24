cmake_minimum_required(VERSION 2.6)

# Name the project
project(TDA)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "-fpermissive -std=c++11")
endif(NOT MSVC)

# Find DartLib
find_package(DartLib REQUIRED HINTS "${CMAKE_SOURCE_DIR}/3rdparty")
if (NOT DartLib_FOUND)
    message(FATAL_ERROR "DartLib Not Found!")
endif (NOT DartLib_FOUND)

# Find OpenGL and GLUT
if(MSVC)
  find_package(freeglut REQUIRED HINTS "${CMAKE_SOURCE_DIR}/3rdparty")
  if (NOT freeglut_FOUND)
    message(FATAL_ERROR "freeglut Not Found!")
  endif (NOT freeglut_FOUND)
else(MSVC)
  find_package(OpenGL REQUIRED)
  find_package(GLUT REQUIRED)
endif(MSVC)

# Add module
add_subdirectory("handle_tunnel_loop")
