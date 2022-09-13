include_guard_x()

# Use external package
find_package(draco 1.5.2 REQUIRED)
add_library(draco ALIAS draco::draco)

# Organize our build
set_target_properties(draco::draco PROPERTIES FOLDER deps)
