include_guard_x()

add_library(xatlas ${CMAKE_SOURCE_DIR}/deps/xatlas/source/xatlas/xatlas.cpp)
target_include_directories(xatlas SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/deps/xatlas/source/xatlas)