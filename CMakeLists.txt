cmake_minimum_required(VERSION 3.2)
project(INF251 CXX)

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/extern")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Handle Mac's Clang
if(APPLE)
  set(CMAKE_CXX_FLAGS "-std=c++14 -stdlib=libc++")
endif()

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
elseif(APPLE)
  set (CMAKE_PREFIX_PATH "~/Qt/5.7/clang_64")
endif()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_package(Qt5Widgets)

##------------------------------------------------------------------------------
## Platform-specific stuff
##

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND LIBRARIES dl pthread)
endif()

##------------------------------------------------------------------------------
## Sources
##

set(SOURCES
  source/infdef.hh
  source/main.cc
  source/CameraPath.cc
  source/CameraPath.hh
  source/Debug.cc
  source/Object.cc
  source/Object.hh
  source/Texture.cc
  source/Texture.hh
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
  source/Cubemap.cc
  source/Cubemap.hh
  )

set(UI
  source/MainWindow.cc
  source/MainWindow.hh
  source/HelpDialog.cc
  source/HelpDialog.hh
  source/LightDialog.cc
  source/LightDialog.hh
  source/LightDialog.ui
)

set(SHADERS
  resources/shaders/basic.fs.glsl
  resources/shaders/depth.fs.glsl
  resources/shaders/grid.fs.glsl
  resources/shaders/toon.fs.glsl
  resources/shaders/lines.fs.glsl
  )

set(RESOURCES
  resources/images/images.qrc
  resources/textures/textures0.qrc
  resources/textures/textures1.qrc
  resources/shaders/shaders.qrc
  resources/meshes/meshes.qrc
  resources/qdarkstyle/style.qrc
  )

##------------------------------------------------------------------------------
## Targets
##

if(MSVC)
  add_executable(inf251 ${SOURCES} ${SHADERS} ${UI} ${RESOURCES})
else()
  add_executable(inf251 ${SOURCES} ${UI} ${RESOURCES})
endif()
target_link_libraries(inf251 ${LIBRARIES})
target_link_libraries(inf251 Qt5::Widgets)
target_include_directories(inf251 PRIVATE ${INCLUDE_DIRS})
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
  set_target_properties(inf251 PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
  set_target_properties(inf251 PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")

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
