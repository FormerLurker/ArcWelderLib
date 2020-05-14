cmake_minimum_required (VERSION "3.13")

project(ArcWelderInverseProcessor C CXX)

# add definitions from the GcodeProcessorLib and ArcWelder libraries
add_definitions(${GcodeProcessorLib_DEFINITIONS} ${ArcWelder_DEFINITIONS})

# Include the GcodeProcessorLib and ArcWelder's directories
include_directories(${GcodeProcessorLib_INCLUDE_DIRS} ${ArcWelder_INCLUDE_DIRS})

# include sourcelist.cmake, which contains our source list and exposes it as the
# ArcWelderConsoleSources variable
include(sourcelist.cmake)

# Add an executable our ArcWelderConsoleSources variable from our sourcelist file
add_executable(${PROJECT_NAME} ${ArcWelderInverseProcessorSources})

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION bin
)


# specify linking to the GcodeProcessorLib and ArcWelder libraries
target_link_libraries(${PROJECT_NAME} GcodeProcessorLib ArcWelder)
