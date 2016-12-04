cmake_minimum_required(VERSION 3.2)
project(GRIEG CXX)

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
  source/Camera.cc
  source/Camera.hh
  source/Debug.cc
  source/Object.cc
  source/Object.hh
  source/Texture.cc
  source/Texture.hh
  source/Shader.cc
  source/Shader.hh
  source/ShaderStorage.hh
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
  resources/shaders/ambient.fs.glsl
  resources/shaders/normals.fs.glsl
  resources/shaders/basic.fs.glsl
  resources/shaders/depth.fs.glsl
  resources/shaders/fog.fs.glsl
  resources/shaders/height.fs.glsl
  resources/shaders/grid.fs.glsl
  resources/shaders/toon.fs.glsl
  resources/shaders/lines.fs.glsl
  resources/shaders/skybox.fs.glsl
  resources/shaders/skybox.vs.glsl
  )

set(RESOURCES
  resources/images/images.qrc
  resources/shaders/shaders.qrc
  resources/qdarkstyle/style.qrc
  )

##------------------------------------------------------------------------------
## Resources
##

add_custom_target(always ALL DEPENDS copy_resources)
add_custom_command(
  OUTPUT copy_resources
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/resources
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/textures ${CMAKE_BINARY_DIR}/resources/textures
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/meshes ${CMAKE_BINARY_DIR}/resources/meshes
  DEPENDS always_execute
  )
add_custom_command(OUTPUT always_execute COMMAND ${CMAKE_COMMAND} -E echo)

##------------------------------------------------------------------------------
## Targets
##

if(MSVC)
  add_executable(grieg ${SOURCES} ${SHADERS} ${UI} ${RESOURCES})
else()
  add_executable(grieg ${SOURCES} ${UI} ${RESOURCES})
endif()
target_link_libraries(grieg ${LIBRARIES})
target_link_libraries(grieg Qt5::Widgets)
target_include_directories(grieg PRIVATE ${INCLUDE_DIRS})
set_target_properties(grieg PROPERTIES CXX_LANGUAGE_STANDARD 11)
add_definitions(-DSPHERICAL_TRACKBALL)
add_dependencies(grieg always)

##------------------------------------------------------------------------------
## MSVC specifics
##

if(MSVC)
  source_group("Shaders" FILES ${SHADERS})
  source_group("Ui" FILES ${UI})
  source_group("Resources" FILES ${RESOURCES})

  if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.5)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT grieg)
  endif()
endif()

if(WIN32)
  set_target_properties(grieg PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
  set_target_properties(grieg PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")

  #if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    ## 32 bits
    #add_custom_command(TARGET grieg POST_BUILD
      #COMMAND ${CMAKE_COMMAND} -E copy_directory
      #${CMAKE_SOURCE_DIR}/extern/lib/x86
      #${CMAKE_BINARY_DIR})
    #
    #else()
    ## 64 bits
    add_custom_command(TARGET grieg POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/extern/lib/x64
      ${CMAKE_BINARY_DIR})
    #endif()
endif()
