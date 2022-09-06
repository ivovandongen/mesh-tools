
macro(INCLUDE_VENDOR_PKG MODULE_NAME)

    # Ensure the git module is up to date
    git_update(${MODULE_NAME})

    # Include the module's cmake file
    include(${CMAKE_SOURCE_DIR}/deps/${MODULE_NAME}.cmake)
endmacro()