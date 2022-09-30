include_guard_x()

if (TARGET glm)
    return()
endif ()

add_library(glm INTERFACE)
target_include_directories(glm SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/deps/glm)

# Organize our build
set_target_properties(glm PROPERTIES FOLDER deps)

install(TARGETS glm EXPORT MeshtoolsDependenciesTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )