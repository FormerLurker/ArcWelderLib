cmake_minimum_required (VERSION "3.15")
set(CMAKE_VERBOSE_MAKEFILE ON)

# You can tweak some common (for all subprojects) stuff here. For example:
project(Build C CXX)

# Use and require C++ 11
set (CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
option(USE_CXX_EXCEPTIONS "Enable C++ exception support" ON)

# Add -O0 to remove optimizations when using gcc
IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)


if(MSVC)
    # link to the msvc runtime statically, keeping debug info if we are in debug config
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# add a definition so our libraries know that the version info is available
add_definitions("-DHAS_GENERATED_VERSION")
# include the generated header.
include_directories("${CMAKE_BINARY_DIR}/GcodeProcessorLib/generated/")

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

# Fix behavior of CMAKE_CXX_STANDARD when targeting macOS.
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()

# add subdirectories to compile in order of inheritance
add_subdirectory(${CMAKE_SOURCE_DIR}/TCLAP)
add_subdirectory(${CMAKE_SOURCE_DIR}/GcodeProcessorLib)
add_subdirectory(${CMAKE_SOURCE_DIR}/ArcWelder)
add_subdirectory(${CMAKE_SOURCE_DIR}/ArcWelderConsole)
#include(${CMAKE_CURRENT_SOURCE_DIR}/ArcWelderConsole/CMakeLists.txt)
add_subdirectory(${CMAKE_SOURCE_DIR}/ArcWelderInverseProcessor)
add_subdirectory(${CMAKE_SOURCE_DIR}/PyArcWelder)





