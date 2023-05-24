cmake_minimum_required(VERSION 3.0.0)
project(networkProgramming VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(networkProgramming src/main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

option(ENABLE_FULL_STRESS_TESTS
       "Build all stress tests and include in test list" OFF)
if (ENABLE_FULL_STRESS_TESTS)
  message("ENABLE_FULL_STRESS_TESTS set - building all stress tests")
else ()
  message("ENABLE_FULL_STRESS_TESTS not set - building some stress tests")
endif ()

add_subdirectory(tests)
