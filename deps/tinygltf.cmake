include_guard_x()

add_library(tinygltf INTERFACE)
target_include_directories(tinygltf SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/deps/tinygltf)

# Organize our build
set_target_properties(tinygltf PROPERTIES FOLDER deps)
