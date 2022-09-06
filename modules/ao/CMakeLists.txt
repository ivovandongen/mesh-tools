add_module(ao)

include_vendor_pkg(embree)
include_vendor_pkg(xatlas)

target_link_libraries(ao PUBLIC core models embree xatlas)

cpp_as_objcpp(${CMAKE_CURRENT_SOURCE_DIR})