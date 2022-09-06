include_guard_x()

add_library(tinyobjloader STATIC ${CMAKE_SOURCE_DIR}/deps/tinyobjloader/tiny_obj_loader.cc)
target_include_directories(tinyobjloader SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/deps/tinyobjloader)
set_property(TARGET tinyobjloader PROPERTY CXX_STANDARD 11)

# Organize our build
set_target_properties(tinyobjloader PROPERTIES FOLDER deps)
