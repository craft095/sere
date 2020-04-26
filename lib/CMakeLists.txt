# minimum required CMAKE version
CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)

project(SereLibrary)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR})

add_definitions(-DANTLR4CPP_STATIC)
set(ANTLR4_WITH_STATIC_CRT OFF)
include_directories(${ANTLR4_INCLUDE_DIRS})
set(ANTLR_EXECUTABLE /home/dima/bin/antlr-4.8-complete.jar)
find_package(ANTLR REQUIRED)

antlr_target(
  SereParser
  Sere.g4
  PACKAGE ast
  VISITOR
  LISTENER)
add_custom_target(GenerateParser DEPENDS ${ANTLR_SereParser_CXX_OUTPUTS})

include_directories(${extern_antlr4_SOURCE_DIR}/runtime/Cpp/runtime/src)
include_directories(${ANTLR_SereParser_OUTPUT_DIR})

file (GLOB sere_files "ast/*.cpp" "boolean/*.cpp" "nfasl/*.cpp" "rt/*.cpp" "sat/*.cpp")
add_library(sere_shared SHARED ${sere_files} ${ANTLR_SereParser_CXX_OUTPUTS})
add_library(sere_static STATIC ${sere_files} ${ANTLR_SereParser_CXX_OUTPUTS})

target_include_directories(sere_static PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(sere_shared PUBLIC ${PROJECT_SOURCE_DIR})

target_compile_options(sere_shared PUBLIC -fPIC -Wno-attributes)
target_compile_options(sere_static PUBLIC -fPIC -Wno-attributes)

add_dependencies(sere_static GenerateParser)
add_dependencies(sere_shared GenerateParser)

target_link_libraries(
  sere_shared
  PRIVATE
  minisat_static
  nlohmann_json::nlohmann_json
  antlr4_static
  z3
  )

target_link_libraries(
  sere_static
  PRIVATE
  minisat_static
  nlohmann_json::nlohmann_json
  antlr4_static
  z3
  )
