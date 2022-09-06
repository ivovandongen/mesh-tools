add_module(ao)

include_vendor_pkg(embree)

target_link_libraries(ao PUBLIC core models uvmap PRIVATE embree)

cpp_as_objcpp(${CMAKE_CURRENT_SOURCE_DIR})