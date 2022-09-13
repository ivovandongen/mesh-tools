add_module(models)

include_vendor_pkg(draco)
include_vendor_pkg(tinygltf)
include_vendor_pkg(tinyobjloader)

target_link_libraries(models PUBLIC core PRIVATE tinygltf tinyobjloader draco)

cpp_as_objcpp(${CMAKE_CURRENT_SOURCE_DIR})