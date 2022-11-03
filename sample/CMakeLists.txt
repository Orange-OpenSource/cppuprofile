PROJECT(uprof-sample DESCRIPTION "Sample using uprofile library")
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

SET( CMAKE_USE_RELATIVE_PATHS ON)

ADD_DEFINITIONS( -std=c++0x )
ADD_COMPILE_OPTIONS("-Wall")
ADD_COMPILE_OPTIONS("-Werror")

SET(Sample_SRCS
    main.cpp
)

ADD_EXECUTABLE(${PROJECT_NAME}
    ${Sample_SRCS}
)

# Specify here the libraries this program depends on
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    cppuprofile
)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin)

