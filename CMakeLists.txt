cmake_minimum_required (VERSION "3.16")
# You can tweak some common (for all subprojects) stuff here. For example:
project(Build C CXX)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

# add subdirectories to compile in order of inheritance
add_subdirectory(${CMAKE_SOURCE_DIR}/GcodeProcessorLib)
add_subdirectory(${CMAKE_SOURCE_DIR}/ArcWelder)
add_subdirectory(${CMAKE_SOURCE_DIR}/ArcWelderConsole)
add_subdirectory(${CMAKE_SOURCE_DIR}/ArcWelderInverseProcessor)
add_subdirectory(${CMAKE_SOURCE_DIR}/PyArcWelder)



