cmake_minimum_required(VERSION 2.8)

project(04_gauss_newton)

add_definitions(-DPATH_ROOT_DIR="${PROJECT_SOURCE_DIR}/..")

####################################

# glfw
IF(MSVC)
  set(GLFW_LIBRARY    ../3rd_party/glfw/src/Release/glfw3.lib)
  set(CMAKE_CXX_FLAGS "-DCMAKE_OSX_ARCHITECTURES=x86_64")
ELSE()
  set(GLFW_LIBRARY    ../3rd_party/glfw/src/libglfw3.a)
ENDIF()
get_filename_component(GLFW_LIBRARY ${GLFW_LIBRARY} ABSOLUTE)

# opengl
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)

#####################################

include_directories(${OpenCV_INCLUDE_DIRS}
                    ../3rd_party/glfw/include
                    ../3rd_party
                    ../src)

add_executable(${PROJECT_NAME}
  main.cpp
  ../src/numOpt.cpp)
  
#####################################

IF(MSVC)
ELSE()
  set(CMAKE_CXX_FLAGS "-Wall -Wno-deprecated-declarations -g")
ENDIF()

IF(APPLE)
  find_library( COCOA_LIBRARY Cocoa )
  find_library( IOKIT_LIBRARY IOKit )
  find_library( COREVIDEO_LIBRARY CoreVideo )
  target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARY}
    ${GLFW_LIBRARY}
    ${COCOA_LIBRARY}
    ${IOKIT_LIBRARY}
    ${COREVIDEO_LIBRARY}
    ${OpenCV_LIBS}
  )
ELSEIF(UNIX)
  target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARY}
    ${GLFW_LIBRARY}
    -lrt -lm -ldl -lX11 -lpthread -lxcb -lXau -lXdmcp)
ELSEIF(MSVC)
  target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARY}
    ${GLFW_LIBRARY})
ENDIF()
    
