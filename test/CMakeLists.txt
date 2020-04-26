cmake_minimum_required(VERSION 3.14)

project(SereTest VERSION 0.9.1)

file (GLOB SereTest_files "*.cpp")

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
