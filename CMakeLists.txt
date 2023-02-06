PROJECT(cppuprofile)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

OPTION(PROFILE_ENABLED "Whether library performs profiling operations or does nothing" ON)
OPTION(SAMPLE_ENABLED "Whether sample binary is built or not" OFF)
OPTION(BUILD_SHARED_LIBS "Build shared libraries" ON)

ADD_SUBDIRECTORY(lib)

IF(SAMPLE_ENABLED)
  ADD_SUBDIRECTORY(sample)
ENDIF()
