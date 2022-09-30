include_guard_x()

if (TARGET stb)
    return()
endif ()

add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/deps/stb)

# Organize our build
set_target_properties(stb PROPERTIES FOLDER deps)

install(TARGETS stb EXPORT MeshtoolsDependenciesTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )
