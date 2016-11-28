cmake_minimum_required(VERSION 3.2)
project(INF251 CXX)

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/extern")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# cmake-ide.el is stupid, so pretend we're doing:
# add_definitions("-std=c++11")

##------------------------------------------------------------------------------
## Dependencies
##

find_package(OpenGL REQUIRED)
list(APPEND INCLUDE_DIRS ${OPENGL_INCLUDE_DIRS})
list(APPEND LIBRARIES ${OPENGL_LIBRARIES})

add_subdirectory(fmtlib)
list(APPEND INCLUDE_DIRS fmtlib)
list(APPEND LIBRARIES fmt)

# OpenGL Math library
list(APPEND INCLUDE_DIRS glm)

# QT
if (MSVC)
  # GOTTA FIX THIS!! CMake keeps pointing to 32bit version of QT
  # and this was the only way I found to force a 64bit linkage
  set (CMAKE_PREFIX_PATH "C:\\Qt\\Qt5.7.0\\5.7\\msvc2015_64\\")
endif()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5Widgets)

##------------------------------------------------------------------------------
## Platform-specific stuff
##

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND LIBRARIES dl)
endif()

##------------------------------------------------------------------------------
## Sources
##

set(SOURCES
  source/infdef.hh
  source/main.cc
  source/Debug.cc
  source/Object.cc
  source/Object.hh
  source/Texture.cc
  source/Texture.hh
  source/Sdl.cc
  source/Sdl.hh
  source/Shader.cc
  source/Shader.hh
  source/ShaderStorage.hh
  source/Text.cc
  source/Text.hh
  source/Trackball.cc
  source/Trackball.hh
  source/Renderer.cc
  source/Renderer.hh
  source/BinParser.hh
  )

set(UI
  source/MainWindow.cc
  source/MainWindow.hh
  source/HelpDialog.cc
  source/HelpDialog.hh
)

set(SHADERS
  resources/shaders/basic.vs.glsl
  resources/shaders/basic.fs.glsl
  resources/shaders/depth.vs.glsl
  resources/shaders/depth.fs.glsl
  resources/shaders/toon.vs.glsl
  resources/shaders/toon.fs.glsl
  resources/shaders/grid.vs.glsl
  resources/shaders/grid.fs.glsl
  resources/shaders/text.vs.glsl
  resources/shaders/text.fs.glsl
  )

set(RESOURCES
  resources/images/images.qrc
  resources/textures/textures0.qrc
  resources/textures/textures1.qrc
  resources/shaders/shaders.qrc
  resources/qdarkstyle/style.qrc
  )

##------------------------------------------------------------------------------
## Targets
##

add_custom_target(assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets)

if(MSVC)
  add_executable(inf251 ${SOURCES} ${SHADERS} ${UI} ${RESOURCES})
else()
  add_executable(inf251 ${SOURCES} ${UI} ${RESOURCES})
endif()
target_link_libraries(inf251 ${LIBRARIES})
target_link_libraries(inf251 Qt5::Widgets)
target_include_directories(inf251 PRIVATE ${INCLUDE_DIRS})
add_dependencies(inf251 assets)
set_target_properties(inf251 PROPERTIES CXX_LANGUAGE_STANDARD 11)
add_definitions(-DSPHERICAL_TRACKBALL)

##------------------------------------------------------------------------------
## MSVC specifics
##

if(MSVC)
  source_group("Shaders" FILES ${SHADERS})
  source_group("Ui" FILES ${UI})
  source_group("Resources" FILES ${RESOURCES})

  if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.5)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT inf251)
  endif()
endif()

if(WIN32)
  #if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    ## 32 bits
    #add_custom_command(TARGET inf251 POST_BUILD
      #COMMAND ${CMAKE_COMMAND} -E copy_directory
      #${CMAKE_SOURCE_DIR}/extern/lib/x86
      #${CMAKE_BINARY_DIR})
    #
    #else()
    ## 64 bits
    add_custom_command(TARGET inf251 POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/extern/lib/x64
      ${CMAKE_BINARY_DIR})
    #endif()
endif()
