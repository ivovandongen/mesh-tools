add_module(uvmap)

include_vendor_pkg(xatlas)

meshtools_module_link_libraries(TARGET uvmap PUBLIC Meshtools::core Meshtools::models PRIVATE xatlas)

cpp_as_objcpp(${CMAKE_CURRENT_SOURCE_DIR})