project(ArcWelder C CXX)

# add definitions from the GcodeProcessorLib project
add_definitions(${GcodeProcessorLib_DEFINITIONS})

# Include the GcodeProcessorLib's directories
include_directories(${GcodeProcessorLib_INCLUDE_DIRS})

# include sourcelist.cmake, which contains our source list and exposes it as the
# ArcWelderSources variable
include(sourcelist.cmake)

# Add a library using our ArcWelderSources variable from our sourcelist file
add_library(${PROJECT_NAME} STATIC ${ArcWelderSources})

install(
    TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib  # <-- Add this line
    COMPONENT library
)

# Link the GcodeProcessorLib
target_link_libraries(${PROJECT_NAME} GcodeProcessorLib)

# Expose the GcodeProcessorLib's Definitions
set(${PROJECT_NAME}_DEFINITIONS ${GcodeProcessorLib_DEFINITIONS}
    CACHE INTERNAL "${PROJECT_NAME}: Definitions" FORCE)

# Expose both the ArcWelder and GcodeProcessorLib's public includes
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/
                                 ${GcodeProcessorLib_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

    