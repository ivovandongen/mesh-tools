include_guard_x()

# Use external package
find_package(embree 3.0 REQUIRED)

# Organize our build
set_target_properties(embree PROPERTIES FOLDER deps)
