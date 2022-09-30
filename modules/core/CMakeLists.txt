add_module(core)

include_vendor_pkg(glm)
include_vendor_pkg(spdlog)
include_vendor_pkg(stb)

meshtools_module_link_libraries(TARGET core PUBLIC spdlog glm PRIVATE stb)
