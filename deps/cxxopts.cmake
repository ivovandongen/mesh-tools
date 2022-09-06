include_guard_x()

add_library(cxxopts INTERFACE)
target_include_directories(cxxopts SYSTEM INTERFACE "${CMAKE_SOURCE_DIR}/deps/cxxopts/include")