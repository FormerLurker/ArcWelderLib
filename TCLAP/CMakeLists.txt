project(TCLAP C CXX)

# Add a library using our ArcWelderSources variable from our sourcelist file
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(TCLAP INTERFACE /tclap)

# Expose both TCLAP's public includes
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)