cmake_minimum_required(VERSION 3.14)

project(SereTest VERSION 0.9.1)

#file (GLOB SereTest_files "*.cpp")

set(Benchmark_files Benchmark.cpp)

set(SereTest_files
  EvalBoolExpr.cpp
  EvalDfasl.cpp
  EvalExpr.cpp
  EvalNfasl.cpp
  EvalRt.cpp
  EvalSere.cpp
  GenDfasl.cpp
  GenNfasl.cpp
  Letter.cpp
  Main.cpp
  TestApi.cpp
  TestDfasl.cpp
  TestExpr.cpp
  TestExtended.cpp
  TestNfasl.cpp
  TestParser.cpp
  TestRt.cpp
  TestSere.cpp
  ToolsZ3.cpp
)

add_executable(sere_benchmark ${Benchmark_files})
target_link_libraries(sere_benchmark PRIVATE
  sere_shared
  nlohmann_json::nlohmann_json
  benchmark)

add_executable(sere_test ${SereTest_files})
target_include_directories(sere_test PUBLIC
                          "${PROJECT_SOURCE_DIR}/.."
                          )
target_link_libraries(
  sere_test PRIVATE
  sere_shared
  z3
  Catch2::Catch
  nlohmann_json::nlohmann_json)

add_test (NAME sere_test COMMAND sere_test)
