cmake_minimum_required(VERSION 3.14)

project(Sere VERSION 0.9.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

enable_testing()

include(FetchContent)

FetchContent_Declare(
  extern_minisat
  GIT_REPOSITORY https://github.com/agurfinkel/minisat
  GIT_TAG        master
)
FetchContent_GetProperties(extern_minisat)

if(NOT extern_minisat_POPULATED)
  FetchContent_Populate(extern_minisat)
  set(MINISAT_SOURCE ${extern_minisat_SOURCE_DIR})
  message(STATUS "Populating minisat " ${extern_minisat_SOURCE_DIR})
  add_subdirectory(minisat EXCLUDE_FROM_ALL)
endif()
message(STATUS "minisat populated " ${extern_minisat_SOURCE_DIR})

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
  extern_robin_map
  GIT_REPOSITORY https://github.com/Tessil/robin-map
  GIT_TAG        v0.6.2
)

FetchContent_GetProperties(extern_robin_map)
if(NOT extern_robin_map_POPULATED)
  FetchContent_Populate(extern_robin_map)
  add_subdirectory(
    ${extern_robin_map_SOURCE_DIR}
    ${extern_robin_map_BINARY_DIR}
    EXCLUDE_FROM_ALL)
endif()
message(STATUS "robin-map populated " ${extern_robin_map_SOURCE_DIR})

FetchContent_Declare(
  extern_json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG        v3.7.3
)

FetchContent_GetProperties(extern_json)
if(NOT extern_json_POPULATED)
  FetchContent_Populate(extern_json)
  set(JSON_BuildTests "OFF")
  add_subdirectory(
    ${extern_json_SOURCE_DIR}
    ${extern_json_BINARY_DIR}
    EXCLUDE_FROM_ALL)
endif()
message(STATUS "json populated " ${extern_json_SOURCE_DIR})

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
message(STATUS "catch2 populated " ${extern_catch_SOURCE_DIR})

##############

FetchContent_Declare(
  extern_benchmark
  GIT_REPOSITORY https://github.com/google/benchmark
  GIT_TAG        v1.5.0
)
FetchContent_GetProperties(extern_benchmark)
if(NOT extern_benchmark_POPULATED)
  message(STATUS "Populating benchmark")
  message(STATUS "benchmark populating: " ${extern_benchmark_SOURCE_DIR})
  message(STATUS "benchmark populating: " ${extern_benchmark_BINARY_DIR})
  FetchContent_Populate(extern_benchmark)
  set(BENCHMARK_ENABLE_TESTING OFF)
  add_subdirectory(
    ${extern_benchmark_SOURCE_DIR}
    ${extern_benchmark_BINARY_DIR}
    EXCLUDE_FROM_ALL)
endif()
message(STATUS "benchmark populated " ${extern_benchmark_SOURCE_DIR})

#############

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
target_compile_options(antlr4_static PRIVATE "-fPIC")
set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/antlr-4.8-complete.jar)
message(STATUS "antlr4 populated " ${extern_antlr4_SOURCE_DIR})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${extern_antlr4_SOURCE_DIR}/runtime/Cpp/cmake)

configure_file(SereLibConfig.h.in SereLibConfig.h)

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

add_subdirectory(lib)
add_subdirectory(compute)
add_subdirectory(tool)
add_subdirectory(python)
add_subdirectory(test)
