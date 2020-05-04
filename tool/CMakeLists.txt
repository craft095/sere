cmake_minimum_required(VERSION 3.14)

project(SereTool VERSION 0.9.1)

file (GLOB SereTool_files "*.cpp")

add_executable(sere_tool ${SereTool_files})
target_include_directories(sere_tool PUBLIC
                          "${PROJECT_SOURCE_DIR}/.."
                          )
target_link_libraries(
  sere_tool PRIVATE
  sere_static)
