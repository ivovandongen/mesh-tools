add_module(ao-cli)

include_vendor_pkg(cxxopts)

meshtools_module_link_libraries(TARGET ao-cli PUBLIC Meshtools::core PRIVATE Meshtools::uvmap Meshtools::ao cxxopts)

cpp_as_objcpp(${CMAKE_CURRENT_SOURCE_DIR})