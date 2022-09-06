add_module(uvmap)

include_vendor_pkg(xatlas)

target_link_libraries(uvmap PUBLIC core models PRIVATE xatlas)

cpp_as_objcpp(${CMAKE_CURRENT_SOURCE_DIR})