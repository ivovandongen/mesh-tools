add_module(models)

include_vendor_pkg(tinygltf)
include_vendor_pkg(tinyobjloader)

#XXX for debug purposes
include_vendor_pkg(xatlas)

target_link_libraries(models PUBLIC core tinygltf tinyobjloader xatlas)

cpp_as_objcpp(${CMAKE_CURRENT_SOURCE_DIR})