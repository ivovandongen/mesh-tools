include_vendor_pkg(googletest)

file(GLOB dirs "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach (dir ${dirs})
    if (IS_DIRECTORY ${dir})
        # Add the test module
        add_subdirectory(${dir})
    endif ()
endforeach ()
