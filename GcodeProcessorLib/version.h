#ifndef VERSION_H
  #define VERSION_H
    #ifndef HAS_GENERATED_VERSION
    #define VERSION_GENERATED_H
    #define GIT_BRANCH "master"
    #define GIT_COMMIT_HASH "11d11e4"
    #define GIT_TAGGED_VERSION "1.1.0"
    #define GIT_TAG "1.1.0"
    #define BUILD_DATE "2021-01-24T20:44:10Z"
    #define COPYRIGHT_DATE "2021"
    #define AUTHOR "Brad Hochgesang"
  #else
      #include "version.generated.h"
  #endif
#endif