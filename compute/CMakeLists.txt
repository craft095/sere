cmake_minimum_required(VERSION 3.14)

project(SereCompute VERSION 0.9.1)

include_directories(${PROJECT_SOURCE_DIR}/../lib)

add_definitions(-DANTLR4CPP_STATIC)
set(ANTLR4_WITH_STATIC_CRT OFF)
include_directories(${ANTLR4_INCLUDE_DIRS})
find_package(ANTLR REQUIRED)

antlr_target(
  ComputeLexer
  ComputeLexer.g4 LEXER
  PACKAGE compute
  )
antlr_target(
  ComputeParser
  ComputeParser.g4 PARSER
  PACKAGE compute
  DEPEND_ANTLR ComputeLexer
  COMPILE_FLAGS -lib ${ANTLR_ComputeLexer_OUTPUT_DIR}
  VISITOR
  LISTENER
  )
add_custom_target(
  ComputeParser
  DEPENDS
  ${ANTLR_ComputeLexer_CXX_OUTPUTS}
  ${ANTLR_ComputeParser_CXX_OUTPUTS})

include_directories(${extern_antlr4_SOURCE_DIR}/runtime/Cpp/runtime/src)
include_directories(${ANTLR_ComputeLexer_OUTPUT_DIR})
include_directories(${ANTLR_ComputeParser_OUTPUT_DIR})

# file (GLOB SereCompute_files "Parser.cpp")

# file (GLOB sere_files
#   "api/*.cpp" "ast/*.cpp" "boolean/*.cpp" "nfasl/*.cpp" "rt/*.cpp" "sat/*.cpp")
# file (GLOB sere_public_headers "Match.hpp")
# add_library(sere_shared SHARED ${sere_files} ${ANTLR_SereParser_CXX_OUTPUTS})
# add_library(compute_static STATIC ${SereCompute_files} ${ANTLR_ComputeParser_CXX_OUTPUTS})
add_library(
  compute_static
  STATIC
  "Parser.cpp"
  "Error.cpp"
  "Ops.cpp"
  "Ast.cpp"
  "Typed.cpp"
  "Infer.cpp"
  "NameContext.cpp"
  ${ANTLR_ComputeLexer_CXX_OUTPUTS}
  ${ANTLR_ComputeParser_CXX_OUTPUTS}
  )

target_include_directories(compute_static PUBLIC ${PROJECT_SOURCE_DIR})
# target_include_directories(sere_shared PUBLIC ${PROJECT_SOURCE_DIR})

# target_compile_options(sere_shared PUBLIC -fPIC -Wno-attributes)
target_compile_options(compute_static PUBLIC -fPIC -Wno-attributes)

add_dependencies(
  compute_static
  ComputeParser
  )
# add_dependencies(sere_shared ComputeParser)

target_link_libraries(
  compute_static
  PRIVATE
  sere_static
  nlohmann_json::nlohmann_json
)

# target_link_libraries(
#   sere_shared
#   PRIVATE
#   minisat_static
#   tsl::robin_map
#   nlohmann_json::nlohmann_json
#   antlr4_static
#   z3
#   )
