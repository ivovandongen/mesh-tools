include_guard_x()

if (TARGET cxxopts)
    return()
endif ()

add_library(cxxopts INTERFACE)
target_include_directories(cxxopts SYSTEM INTERFACE "${PROJECT_SOURCE_DIR}/deps/cxxopts/include")

install(TARGETS cxxopts EXPORT MeshtoolsDependenciesTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )