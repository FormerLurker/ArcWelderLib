cmake_minimum_required (VERSION "3.16")

project(GcodeProcessorLib C CXX)

# include sourcelist.cmake, which contains our source list and exposes it as the
# GcodeProcessorLibSources variable
include(sourcelist.cmake)

# Add a library using our GcodeProcessorLibSources variable from our sourcelist file
add_library(${PROJECT_NAME} STATIC ${GcodeProcessorLibSources})

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME}
                      PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME})

# Expose the public includes via a cache variable
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

