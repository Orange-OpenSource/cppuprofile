PROJECT(cppuprofile DESCRIPTION "CppProfiling library")
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

SET( CMAKE_USE_RELATIVE_PATHS ON)

ADD_DEFINITIONS( -std=c++0x )
ADD_COMPILE_OPTIONS("-Wall")
ADD_COMPILE_OPTIONS("-Werror")

SET(UProfile_HEADERS
    uprofile.h
    uprofileimpl.h
    util/memoryusage.h
)

SET(UProfile_IMPL
    uprofile.cpp
    uprofileimpl.cpp
    util/memoryusage.cpp
)

SET(UProfile_SRCS
    ${UProfile_HEADERS}
    ${UProfile_IMPL}
)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_LIBRARY(${LIBRARY_NAME} SHARED
    ${UProfile_SRCS}
)

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
INSTALL(FILES uprofile.h
    DESTINATION include/${LIBRARY_NAME}
)
INSTALL(TARGETS ${LIBRARY_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
    DESTINATION lib/pkgconfig
)

