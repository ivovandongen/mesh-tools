add_module(ao-cli)

include_vendor_pkg(cxxopts)

target_link_libraries(ao-cli PUBLIC core uvmap ao cxxopts)

cpp_as_objcpp(${CMAKE_CURRENT_SOURCE_DIR})