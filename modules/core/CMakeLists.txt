add_module(core)

include_vendor_pkg(spdlog)
include_vendor_pkg(stb)

target_link_libraries(core PUBLIC spdlog PRIVATE stb)
