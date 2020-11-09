project(HandleTunnelLoop)

if(APPLE)
  add_definitions(-DMAC_OS)
endif(APPLE)

# Include the 'include' of the project itself
include_directories("include")
# Include DartLib core in 3rdparty
include_directories("${DartLib_DIR}/core")

# Include the directory of OpenGL and GLUT
if(MSVC)
  include_directories("${freeglut_DIR}/include")
else(MSVC)
  include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})
endif(MSVC)

# Manually add the sources
# set(SOURCES_ONE ${MeshLib_DIR}/core/bmp/RgbImage.cpp)

# Also, the file(GLOB...) allows for wildcard additions
file(GLOB SRCS
    "include/*.h"
    "src/*.cpp")

# Add an executable target called MyDemo to be build from 
# the source files.
add_executable(HandleTunnelLoop ${SRCS})

# Link the libraries of freeglut
if(MSVC)
  if(CMAKE_CL_64)
    target_link_libraries(HandleTunnelLoop "${freeglut_DIR}/lib/x64/freeglut.lib")
  else(CMAKE_CL_64)
    target_link_libraries(HandleTunnelLoop "${freeglut_DIR}/lib/freeglut.lib")
  endif(CMAKE_CL_64)
else(MSVC)
  target_link_libraries(HandleTunnelLoop ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
endif(MSVC)

# Install the executeable program in the bin folder of the
# current project directory.
install(TARGETS HandleTunnelLoop DESTINATION ${CMAKE_SOURCE_DIR}/bin)
