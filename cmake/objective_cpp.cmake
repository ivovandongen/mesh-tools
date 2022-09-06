macro(CPP_AS_OBJCPP DIR)
    if (APPLE AND ENABLE_METAL)
        message(STATUS "Ensuring implementation files are treated as Obj-C++ for ${DIR}")
        # Need to treat some c++ files as Obj-Cpp for Metal on Apple
        file(GLOB_RECURSE CPP_SRC_FILES
                RELATIVE ${DIR}
                "${DIR}/*.cpp"  "${DIR}/**/*.cpp"
                )
        set_source_files_properties(${CPP_SRC_FILES} PROPERTIES COMPILE_FLAGS "-x objective-c++")
    endif ()
endmacro()