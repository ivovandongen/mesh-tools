add_module(models)

include_vendor_pkg(draco)
include_vendor_pkg(tinygltf)
include_vendor_pkg(tinyobjloader)

meshtools_module_link_libraries(TARGET models PUBLIC Meshtools::core PRIVATE tinygltf tinyobjloader draco::draco)

cpp_as_objcpp(${CMAKE_CURRENT_SOURCE_DIR})