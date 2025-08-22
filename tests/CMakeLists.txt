PROJECT(uprof-tests DESCRIPTION "Unit tests")
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

SET( CMAKE_USE_RELATIVE_PATHS ON)

INCLUDE(FetchContent)

FETCHCONTENT_DECLARE(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1 # or a later release
)

FETCHCONTENT_MAKEAVAILABLE(Catch2)

LIST(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
INCLUDE(CTest)
INCLUDE(Catch)

ADD_EXECUTABLE(${PROJECT_NAME}
  test.cpp
)

target_link_libraries(${PROJECT_NAME}
    Catch2::Catch2WithMain
    cppuprofile
)