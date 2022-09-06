# CCache

if(ENABLE_CCACHE AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        message(STATUS "Using ccache to speed up incremental builds")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
        set(ENV{CCACHE_CPP2} "true")
    endif()
endif()
