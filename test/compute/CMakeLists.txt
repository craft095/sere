cmake_minimum_required(VERSION 3.14)

project(ComputeTest VERSION 0.9.1)

set(ComputeTest_files
#  ../EvalBoolExpr.cpp
#  ../EvalExpr.cpp
#  ../EvalNfasl.cpp
  ../EvalRt.cpp
#  ../EvalSere.cpp
#  ../GenNfasl.cpp
#  ../Letter.cpp
  Main.cpp
  TestParser.cpp
)

add_executable(compute_test ${ComputeTest_files})
target_include_directories(compute_test PUBLIC
                          "${PROJECT_SOURCE_DIR}/../.."
                          )
target_link_libraries(
  compute_test PRIVATE
  compute_static
  sere_static
  Catch2::Catch
  tsl::robin_map
  nlohmann_json::nlohmann_json)

add_test (NAME compute_test COMMAND compute_test)
