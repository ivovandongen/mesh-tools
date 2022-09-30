include_guard_x()

if (TARGET draco)
    return()
endif ()

# Use external package
find_package(draco 1.5.2 REQUIRED)