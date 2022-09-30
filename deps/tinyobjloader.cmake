include_guard_x()

if (TARGET tinyobjloader)
    return()
endif ()

add_library(tinyobjloader STATIC ${PROJECT_SOURCE_DIR}/deps/tinyobjloader/tiny_obj_loader.cc)
target_include_directories(tinyobjloader SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/deps/tinyobjloader)
set_property(TARGET tinyobjloader PROPERTY CXX_STANDARD 11)

# Organize our build
set_target_properties(tinyobjloader PROPERTIES FOLDER deps)

install(TARGETS tinyobjloader EXPORT MeshtoolsDependenciesTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )
