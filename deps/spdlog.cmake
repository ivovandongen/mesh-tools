include_guard_x()

if (TARGET spdlog)
    return()
endif ()

add_library(spdlog INTERFACE)
target_include_directories(spdlog SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/deps/spdlog/include)

# Organize our build
set_target_properties(spdlog PROPERTIES FOLDER deps)

install(TARGETS spdlog EXPORT MeshtoolsDependenciesTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )
