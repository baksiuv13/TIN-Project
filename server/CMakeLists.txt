cmake_minimum_required(VERSION 3.0.0)
project(TIN-Project-Server CXX)
#project(TIN-Project-Server CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

include_directories("${CMAKE_SOURCE_DIR}")

#include(CTest)
#enable_testing()

add_subdirectory(core)
get_directory_property(
  SOURCES_core DIRECTORY core DEFINITION SOURCES
)

add_subdirectory(xd)
get_directory_property(
  SOURCES_xd DIRECTORY xd DEFINITION SOURCES
)

add_subdirectory(account_manager)
get_directory_property(
  SOURCES_account_manager DIRECTORY account_manager DEFINITION SOURCES
)

add_subdirectory(configurator)
get_directory_property(
  SOURCES_configurator DIRECTORY configurator DEFINITION SOURCES
)

add_subdirectory(network)
get_directory_property(
  SOURCES_network DIRECTORY network DEFINITION SOURCES
)

add_subdirectory(app)
get_directory_property(
  SOURCES_app DIRECTORY app DEFINITION SOURCES
)

add_subdirectory(instructions)
get_directory_property(
  SOURCES_instructions DIRECTORY instructions DEFINITION SOURCES
)

add_executable(server
  main.cc
  "${SOURCES_core}"
  "${SOURCES_xd}"
  "${SOURCES_image}"
  "${SOURCES_account_manager}"
  "${SOURCES_configurator}"
  "${SOURCES_network}"
  "${SOURCES_app}"
  "${SOURCES_instructions}"
)

target_link_libraries(server
  pthread
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
#include(CPack)