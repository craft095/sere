# minimum required CMAKE version
CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)

project(libsere)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

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

#target_include_directories(demo PRIVATE ${Z3_CXX_INCLUDE_DIRS})

# add_executable(demo Main.cpp
#                ${ANTLR_SereParser_CXX_OUTPUTS})

# message(STATUS "ANTLR_SereParser_CXX_OUTPUTS: ${ANTLR_SereParser_CXX_OUTPUTS}")
# message(STATUS "Z3_FOUND: ${Z3_FOUND}")
# message(STATUS "Found Z3 ${Z3_VERSION_STRING}")
# message(STATUS "Z3_DIR: ${Z3_DIR}")

file (GLOB libsere_files "ast/*.cpp" "boolean/*.cpp" "nfasl/*.cpp" "rt/*.cpp" "sat/*.cpp")
add_library(libsere_shared SHARED ${libsere_files} ${ANTLR_SereParser_CXX_OUTPUTS})
add_library(libsere_static STATIC ${libsere_files} ${ANTLR_SereParser_CXX_OUTPUTS})

target_compile_options(libsere_shared PUBLIC -fPIC)
target_compile_options(libsere_static PUBLIC -fPIC)
add_dependencies(libsere_static GenerateParser)
add_dependencies(libsere_shared GenerateParser)

target_link_libraries(
  libsere_shared
  PRIVATE
  minisat_static
  nlohmann_json::nlohmann_json
  antlr4_static
  z3
  )

target_link_libraries(
  libsere_static
  PRIVATE
  minisat_static
  nlohmann_json::nlohmann_json
  antlr4_static
  z3
  )
