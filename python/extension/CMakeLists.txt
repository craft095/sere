cmake_minimum_required(VERSION 3.10)

project(PythonSereExtension VERSION 0.9.1)

find_package(PythonInterp 3.5 REQUIRED)
# This goes after, since it uses PythonInterp as hint
find_package(PythonLibs 3.5 REQUIRED)

# Find Python headers
exec_program(${PYTHON_EXECUTABLE}
  ARGS "-c \"import sysconfig; print(sysconfig.get_paths()['include'])\""
  OUTPUT_VARIABLE PYTHON_INCLUDE_DIRS
  RETURN_VALUE PYTHON_INCLUDE_DIRS_NOT_FOUND
  )
if(PYTHON_INCLUDE_DIRS_NOT_FOUND)
  message(FATAL_ERROR "Python headers not found")
endif()

file (GLOB PythonSereExtension_files "*.c")

add_library(serec SHARED ${PythonSereExtension_files})

target_include_directories(serec PUBLIC
    ${PYTHON_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}
    )

set_target_properties(serec
  PROPERTIES
    PREFIX ""
  )
target_link_libraries(serec sere_shared)
