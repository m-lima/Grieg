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

##------------------------------------------------------------------------------
## GLAD
##

set(GLAD_PROFILE "core")
set(GLAD_API "gl=4.5")

# set(GLAD_EXTENSIONS ...)

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
  source/main.cc)

##------------------------------------------------------------------------------
## Targets
##

add_executable(inf251 ${SOURCES})
target_link_libraries(inf251 ${LIBRARIES})
target_include_directories(inf251 PRIVATE ${INCLUDE_DIRS})
set_target_properties(inf251 PROPERTIES CXX_LANGUAGE_STANDARD 11)
