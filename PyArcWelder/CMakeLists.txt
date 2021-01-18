project(PyArcWelder C CXX)

# PythonLibs is required to build a python extension
find_package(PythonLibs REQUIRED)

# Add definitions from ArcWelder and GcodeProcessorLib
add_definitions(${ArcWelder_DEFINITIONS} ${GcodeProcessorLib_DEFINITIONS})

# Include Python, ArcWelder and GcodeProcessorLib
include_directories(${PYTHON_INCLUDE_DIRS} ${ArcWelder_INCLUDE_DIRS} ${GcodeProcessorLib_INCLUDE_DIRS})

# include sourcelist.cmake, which contains our source list and exposes it as the
# PyArcWelderSources variable
include(sourcelist.cmake)

# Create our library
add_library(${PROJECT_NAME} SHARED ${PyArcWelderSources})


install(
    TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib  # <-- Add this line
    COMPONENT library
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        PREFIX ""
        OUTPUT_NAME ${PROJECT_NAME}
        LINKER_LANGUAGE C
)

if(WIN32)
    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        SUFFIX ".pyd"
    )
endif()

# Link to ArcWelder, GcodeProcessorLib and the Python Libraries
target_link_libraries(${PROJECT_NAME} ArcWelder GcodeProcessorLib ${PYTHON_LIBRARIES})

# On Windows, it is required to link to the Python libraries
if(WIN32 OR APPLE)
    target_link_libraries(${PROJECT_NAME} ArcWelder GcodeProcessorLib ${PYTHON_LIBRARIES})
endif()

# Expose the GcodeProcessorLib, and ArcWelder's Definitions.
set(${PROJECT_NAME}_DEFINITIONS ${GcodeProcessorLib_DEFINITIONS}
                                ${ArcWelder_DEFINITIONS}
    CACHE INTERNAL "${PROJECT_NAME}: Definitions" FORCE)

# Expose the GcodeProcessorLib, ArcWelder and PyArcWelder's Definitions.
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/
                                 ${ArcWelder_INCLUDE_DIRS}
                                 ${GcodeProcessorLib_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
     