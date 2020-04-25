cmake_minimum_required(VERSION 3.14)

project(Sere VERSION 0.9.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

FetchContent_Declare(
  extern_minisat
  GIT_REPOSITORY https://github.com/niklasso/minisat
  GIT_TAG        releases/2.2.0
)
FetchContent_GetProperties(extern_minisat)

if(NOT extern_minisat_POPULATED)
  FetchContent_Populate(extern_minisat)
  set(MINISAT_SOURCE ${extern_minisat_SOURCE_DIR})
  message(STATUS "Populating minisat " ${extern_minisat_SOURCE_DIR})
  add_subdirectory(minisat)
endif()

# FetchContent_Declare(
#   extern_z3
#   GIT_REPOSITORY https://github.com/Z3Prover/z3
#   GIT_TAG        z3-4.8.7
# )

# FetchContent_GetProperties(extern_z3)
# if(NOT extern_z3_POPULATED)
#   FetchContent_Populate(extern_z3)
#   add_subdirectory(
#     ${extern_z3_SOURCE_DIR}
#     ${extern_z3_BINARY_DIR}
#     EXCLUDE_FROM_ALL)
# endif()

FetchContent_Declare(
  extern_json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG        v3.7.3
)

FetchContent_GetProperties(extern_json)
if(NOT extern_json_POPULATED)
  FetchContent_Populate(extern_json)
  add_subdirectory(
    ${extern_json_SOURCE_DIR}
    ${extern_json_BINARY_DIR}
    EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(
  catch
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.9.2
)
# FetchContent_GetProperties(catch)
if(NOT catch_POPULATED)
  message(STATUS "Populating catch")
  FetchContent_Populate(catch)
  set(CATCH_INCLUDE_DIR ${catch_SOURCE_DIR}/single_include)
  add_library(Catch2::Catch IMPORTED INTERFACE)
  set_property(TARGET Catch2::Catch PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CATCH_INCLUDE_DIR}")
endif()

FetchContent_Declare(
  extern_antlr4
  GIT_REPOSITORY https://github.com/antlr/antlr4
  GIT_TAG        4.8
)

FetchContent_GetProperties(extern_antlr4)
if(NOT extern_antlr4_POPULATED)
  FetchContent_Populate(extern_antlr4)
  add_subdirectory(
    ${extern_antlr4_SOURCE_DIR}/runtime/Cpp
    ${extern_antlr4_BINARY_DIR}
    EXCLUDE_FROM_ALL)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${extern_antlr4_SOURCE_DIR}/runtime/Cpp/cmake)

configure_file(SereLibConfig.h.in SereLibConfig.h)

add_subdirectory(lib)

file (GLOB SereTests_files "test/*.cpp")
add_executable(SereTests ${SereTests_files})

target_link_libraries(SereTests PUBLIC libsere)

target_include_directories(SereTests PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/lib"
                          )
