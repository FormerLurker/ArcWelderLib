cmake_minimum_required (VERSION "3.13")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

project(TCLAP C CXX)

# Add a library using our ArcWelderSources variable from our sourcelist file
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(TCLAP INTERFACE /tclap)

# Expose both TCLAP's public includes
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)