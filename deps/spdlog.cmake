include_guard_x()

add_library(spdlog INTERFACE)
target_include_directories(spdlog SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/deps/spdlog/include)

# Organize our build
set_target_properties(spdlog PROPERTIES FOLDER deps)
