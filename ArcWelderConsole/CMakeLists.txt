cmake_minimum_required (VERSION "3.13")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

project(ArcWelderConsole C CXX)

# add definitions from the GcodeProcessorLib and ArcWelder libraries
add_definitions(${GcodeProcessorLib_DEFINITIONS} ${ArcWelder_DEFINITIONS})
add_definitions("-DHAS_GENERATED_VERSION")

# Include the GcodeProcessorLib and ArcWelder's directories
include_directories(${GcodeProcessorLib_INCLUDE_DIRS} ${ArcWelder_INCLUDE_DIRS} ${TCLAP_INCLUDE_DIRS})

# include sourcelist.cmake, which contains our source list and exposes it as the
# ArcWelderConsoleSources variable
include(sourcelist.cmake)

if(MSVC)
    # link to the msvc runtime statically, keeping debug info if we are in debug config
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Add an executable our ArcWelderConsoleSources variable from our sourcelist file
add_executable(${PROJECT_NAME} ${ArcWelderConsoleSources})
# change the executable name to ArcWelder or ArcWelder.exe
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ArcWelder")


install(
    TARGETS ${PROJECT_NAME}
    DESTINATION bin
)

# specify linking to the GcodeProcessorLib and ArcWelder libraries
target_link_libraries(${PROJECT_NAME} GcodeProcessorLib ArcWelder TCLAP)





