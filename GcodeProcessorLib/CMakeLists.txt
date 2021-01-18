project(GcodeProcessorLib C CXX)

# create a version information header - version.h
# First get the current branch and store to GIT_BRANCH variable
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# get the current commit version
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# get the tag for the current branch
execute_process(
  COMMAND git describe --abbrev=0 --tags
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_TAG
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# get the most recent tagged version
execute_process(
  COMMAND git describe --tags
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_TAGGED_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# get the build date
string(TIMESTAMP BUILD_DATE "%Y-%m-%dT%H:%M:%SZ" UTC)
# get the copyright date
string(TIMESTAMP BUILD_YEAR "%Y" UTC)

# add a definition so our libraries know that the version info is available
add_definitions("-DHAS_GENERATED_VERSION")

# Generate version.h CMAKE_BINARY_DIR
configure_file(
  "${PROJECT_SOURCE_DIR}/version.generated.h.in"
  "${CMAKE_BINARY_DIR}/GcodeProcessorLib/generated/version.generated.h"
)
include_directories("${CMAKE_BINARY_DIR}/GcodeProcessorLib/generated/")

# copy version.h file to the source folder for later deployment
file(COPY "${CMAKE_BINARY_DIR}/GcodeProcessorLib/generated/version.generated.h" DESTINATION "${PROJECT_SOURCE_DIR}/deploy")
file(RENAME "${PROJECT_SOURCE_DIR}/deploy/version.generated.h" "${PROJECT_SOURCE_DIR}/deploy/version.h")

# include sourcelist.cmake, which contains our source list and exposes it as the
# GcodeProcessorLibSources variable
include(sourcelist.cmake)

# Add a library using our GcodeProcessorLibSources variable from our sourcelist file
add_library(${PROJECT_NAME} STATIC ${GcodeProcessorLibSources})

install(
    TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    COMPONENT library
)

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME}
                      PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME})

# Expose the public includes via a cache variable
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

