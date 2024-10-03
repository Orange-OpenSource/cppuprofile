PROJECT(cppuprofile DESCRIPTION "CppProfiling library")
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

SET( CMAKE_USE_RELATIVE_PATHS ON)

IF(CMAKE_COMPILER_IS_GNUCXX)
	ADD_DEFINITIONS( -std=c++0x )
	ADD_DEFINITIONS( -D_GLIBCXX_USE_NANOSLEEP )
ENDIF()

IF(WIN32)
    add_compile_options(/W4)
ELSE()
    add_compile_options(-Wall -Werror)
ENDIF()

IF(BUILD_SHARED_LIBS)
    IF(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12.0)
        ADD_COMPILE_DEFINITIONS(_UPROFILE_BUILD_SHARED)
    ELSE()
        ADD_DEFINITIONS(-D_UPROFILE_BUILD_SHARED)
	ENDIF()
ENDIF()

IF(PROFILE_ENABLED)
	IF(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12.0)
        ADD_COMPILE_DEFINITIONS(PROFILE_ON)
		ADD_DEFINITIONS( -DPROFILE_ON )
    ELSE()
        ADD_DEFINITIONS(-DPROFILE_ON)
	ENDIF()
ENDIF()

SET(UProfile_PUBLIC_HEADERS
    api.h
    uprofile.h
    timestampunit.h
    igpumonitor.h
)

SET(UProfile_IMPL
    uprofile.cpp
    uprofileimpl.cpp
    eventsfile.h
    eventsfile.cpp
    util/timer.cpp
    util/cpumonitor.cpp
)

IF(GPU_MONITOR_NVIDIA)
   LIST(APPEND UProfile_PUBLIC_HEADERS monitors/nvidiamonitor.h)
   LIST(APPEND UProfile_IMPL monitors/nvidiamonitor.cpp)
ENDIF()

SET(UProfile_HEADERS
    ${UProfile_PUBLIC_HEADERS}
    uprofileimpl.h
    util/timer.h
    util/cpumonitor.h
)

SET(UProfile_SRCS
    ${UProfile_HEADERS}
    ${UProfile_IMPL}
)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_LIBRARY(${LIBRARY_NAME} ${UProfile_SRCS}) # STATIC or SHARED are determined through BUILD_SHARED_LIBS flag

IF(BUILD_SHARED_LIBS AND UNIX)
    # Hide symbols not explicitly tagged for export from the shared library
    TARGET_COMPILE_OPTIONS(${LIBRARY_NAME} PRIVATE "-fvisibility=hidden")
ENDIF()


# Specify the include directories exported by this library
TARGET_INCLUDE_DIRECTORIES(${LIBRARY_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

IF(UNIX)
	TARGET_LINK_LIBRARIES(${LIBRARY_NAME} pthread)
ENDIF()

# Set specific pkg-config variables
SET(PKG_CONFIG_LIBDIR
    "\${prefix}/lib"
)
SET(PKG_CONFIG_INCLUDEDIR
    "\${prefix}/include/${LIBRARY_NAME}"
)
SET(PKG_CONFIG_LIBS
    "-L\${libdir} -l${LIBRARY_NAME}"
)
SET(PKG_CONFIG_CFLAGS
    "-I\${includedir}"
)

# Generate the pkg-config file
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/pkg-config.pc.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
)

# Install library project files
INSTALL(FILES ${UProfile_PUBLIC_HEADERS}
    DESTINATION include/${LIBRARY_NAME}
)
INSTALL(TARGETS ${LIBRARY_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
    DESTINATION lib/pkgconfig
)
