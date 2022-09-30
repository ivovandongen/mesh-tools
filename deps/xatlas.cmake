include_guard_x()

if (TARGET xatlas)
    return()
endif ()

add_library(xatlas ${PROJECT_SOURCE_DIR}/deps/xatlas/source/xatlas/xatlas.cpp)
target_include_directories(xatlas SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/deps/xatlas/source/xatlas)

install(TARGETS xatlas EXPORT MeshtoolsDependenciesTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )