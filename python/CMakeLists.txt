add_subdirectory(extension)

find_program (PYTHON_EXECUTABLE python3)

if (PYTHON_EXECUTABLE)
  add_test (pytest ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/run.py)

  set_tests_properties(pytest PROPERTIES
    ENVIRONMENT
    LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:$ENV{LD_LIBRARY_PATH}
    ENVIRONMENT
    PYTHONPATH=${CMAKE_BINARY_DIR}/lib:${CMAKE_SOURCE_DIR}/python:$ENV{PYTHONPATH}
    )

  add_custom_target(Tutorial
    COMMAND ${CMAKE_COMMAND}
      -E env "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:$ENV{LD_LIBRARY_PATH}"
             "PYTHONPATH=${CMAKE_BINARY_DIR}/lib:${CMAKE_SOURCE_DIR}/python:$ENV{PYTHONPATH}"
      ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/tutorial.py)

endif (PYTHON_EXECUTABLE)
