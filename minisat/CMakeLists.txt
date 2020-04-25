cmake_minimum_required(VERSION 3.5)

project(MiniSat VERSION 2.2 LANGUAGES CXX)

add_library(libminisat STATIC
    ${extern_minisat_SOURCE_DIR}/core/Solver.cc
    ${extern_minisat_SOURCE_DIR}/utils/Options.cc
    ${extern_minisat_SOURCE_DIR}/utils/System.cc
    ${extern_minisat_SOURCE_DIR}/simp/SimpSolver.cc
)

target_compile_features(libminisat
    PRIVATE
      cxx_attributes
      cxx_defaulted_functions
      cxx_deleted_functions
      cxx_final
)

target_compile_options(libminisat PRIVATE -fpermissive -Wno-literal-suffix -fPIC)

target_include_directories(libminisat PUBLIC ${extern_minisat_SOURCE_DIR})

target_compile_definitions(libminisat PUBLIC __STDC_LIMIT_MACROS __STDC_FORMAT_MACROS)

# Also build the two MiniSat executables
add_executable(minisat ${extern_minisat_SOURCE_DIR}/core/Main.cc)
target_link_libraries(minisat libminisat)

add_executable(minisat-simp ${extern_minisat_SOURCE_DIR}/simp/Main.cc)
target_link_libraries(minisat-simp libminisat)
