cmake_minimum_required(VERSION 3.5)

#project(MiniSat VERSION 2.2 LANGUAGES CXX)
project(MiniSat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# find_package(ZLIB)
# include_directories(${ZLIB_INCLUDE_DIR})

add_definitions(-D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)

add_library(minisat_static STATIC
    ${extern_minisat_SOURCE_DIR}/minisat/core/Solver.cc
    ${extern_minisat_SOURCE_DIR}/minisat/utils/Options.cc
    ${extern_minisat_SOURCE_DIR}/minisat/utils/System.cc
    ${extern_minisat_SOURCE_DIR}/minisat/simp/SimpSolver.cc
)
set_target_properties(minisat_static PROPERTIES
    CXX_VISIBILITY_PRESET "hidden"
    VISIBILITY_INLINES_HIDDEN ON
)

# target_compile_features(minisat_static
#     PRIVATE
#       cxx_attributes
#       cxx_defaulted_functions
#       cxx_deleted_functions
#       cxx_final
# )

target_compile_options(minisat_static
  PRIVATE
  -fpermissive
  -Wno-literal-suffix
  -fPIC
)

target_include_directories(minisat_static PUBLIC ${extern_minisat_SOURCE_DIR})
target_compile_definitions(minisat_static PUBLIC __STDC_LIMIT_MACROS __STDC_FORMAT_MACROS)

# Also build the two MiniSat executables
# add_executable(minisat ${extern_minisat_SOURCE_DIR}/minisat/core/Main.cc)
# target_link_libraries(minisat minisat_static)

# add_executable(minisat-simp ${extern_minisat_SOURCE_DIR}/minisat/simp/Main.cc)
# target_link_libraries(minisat-simp minisat_static)
