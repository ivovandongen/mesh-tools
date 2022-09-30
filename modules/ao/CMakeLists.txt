add_module(ao)

include_vendor_pkg(embree)

meshtools_module_link_libraries(TARGET ao PUBLIC Meshtools::core Meshtools::models PRIVATE embree)

cpp_as_objcpp(${CMAKE_CURRENT_SOURCE_DIR})