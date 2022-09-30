include_guard_x()

if (TARGET tinygltf)
    return()
endif ()

add_library(tinygltf INTERFACE)
include_vendor_pkg(draco)
target_link_libraries(tinygltf INTERFACE draco::draco)
target_include_directories(tinygltf SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/deps/tinygltf)

# Organize our build
set_target_properties(tinygltf PROPERTIES FOLDER deps)

install(TARGETS tinygltf EXPORT MeshtoolsDependenciesTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )
