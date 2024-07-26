PROJECT(cppuprofile)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

OPTION(PROFILE_ENABLED "Whether library performs profiling operations or does nothing" ON)
OPTION(SAMPLE_ENABLED "Whether sample binary is built or not" OFF)
OPTION(BUILD_SHARED_LIBS "Build shared libraries" ON)
OPTION(GPU_MONITOR_NVIDIA "Whether NVidiaMonitor class for monitoring NVidia GPUs is compiled and embedded to the library" OFF)

if (GPU_MONITOR_NVIDIA)
  add_definitions(-DGPU_MONITOR_NVIDIA)
endif()

ADD_SUBDIRECTORY(lib)

IF(SAMPLE_ENABLED)
  ADD_SUBDIRECTORY(sample)
ENDIF()
