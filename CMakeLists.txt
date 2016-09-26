cmake_minimum_required(VERSION 3.2)
project(INF251 CXX)

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/extern")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

##------------------------------------------------------------------------------
## Dependencies
##

find_package(OpenGL REQUIRED)
list(APPEND INCLUDE_DIRS ${OPENGL_INCLUDE_DIRS})
list(APPEND LIBRARIES ${OPENGL_LIBRARIES})

find_package(SDL2 REQUIRED)
list(APPEND INCLUDE_DIRS ${SDL2_INCLUDE_DIRS})
list(APPEND LIBRARIES ${SDL2_LIBRARIES})

message(STATUS "${SDL2_LIBRARIES}")

add_subdirectory(fmtlib)
list(APPEND INCLUDE_DIRS fmtlib)
list(APPEND LIBRARIES fmt)

# OpenGL Math library
list(APPEND INCLUDE_DIRS glm)

##------------------------------------------------------------------------------
## GLAD
##

set(GLAD_PROFILE "core")
set(GLAD_API "gl=4.3")
set(GLAD_GENERATOR "c-debug")

# set(GLAD_EXTENSIONS)

string(REPLACE ";" "," GLAD_EXTENSIONS "${GLAD_EXTENSIONS}")

add_subdirectory(glad)
list(APPEND INCLUDE_DIRS ${GLAD_INCLUDE_DIRS})
list(APPEND LIBRARIES ${GLAD_LIBRARIES})

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
  source/Sdl.cc
  source/Sdl.hh
  source/Shader.cc
  source/Shader.hh
  source/Trackball.cc
  source/Trackball.hh
  source/Renderer.cc
  source/Renderer.hh)

set(SHADERS
  assets/shaders/cube.vs.glsl
  assets/shaders/cube.fs.glsl
  assets/shaders/triangle.vs.glsl
  assets/shaders/triangle.fs.glsl)

##------------------------------------------------------------------------------
## Targets
##

add_custom_target(assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets)

add_executable(inf251 ${SOURCES} ${SHADERS})
target_link_libraries(inf251 ${LIBRARIES})
target_include_directories(inf251 PRIVATE ${INCLUDE_DIRS})
add_dependencies(inf251 assets)
set_target_properties(inf251 PROPERTIES CXX_LANGUAGE_STANDARD 11)
add_definitions(-DSPHERICAL_TRACKBALL)

##------------------------------------------------------------------------------
## MSVC specifics
##

if(MSVC)
  source_group("Shaders" FILES ${SHADERS})
endif()

if(WIN32)
  if(MSVC)
    add_custom_command(TARGET inf251 POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_SOURCE_DIR}/extern/lib/x86/SDL2.dll
      ${CMAKE_BINARY_DIR}/Debug/SDL2.dll)
  else()
    add_custom_command(TARGET inf251 POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_SOURCE_DIR}/extern/lib/x86/SDL2.dll
      ${CMAKE_BINARY_DIR}/SDL2.dll)
  endif()
endif()
