# Clang format
if (ENABLE_CLANG_FORMAT)
    find_program(CLANG_FORMAT_CMD NAMES "clang-format" DOC "Path to clang-format executable")
    if (NOT CLANG_FORMAT_CMD)
        message(STATUS "clang-format not found.")
    else ()
        message(STATUS "clang-format found: ${CLANG_FORMAT_CMD}")
        set(CLANG_FORMAT_CMD ${CLANG_FORMAT_CMD})
    endif ()
endif (ENABLE_CLANG_FORMAT)

macro(CLANG_FORMAT DIR)

    if (ENABLE_CLANG_FORMAT AND CLANG_FORMAT_CMD)

        if (NOT TARGET clang-format)
            add_custom_target(clang-format ALL)
            set_target_properties(clang-format PROPERTIES FOLDER clang-format)
        endif()

        file(RELATIVE_PATH DIR_SAFE ${PROJECT_SOURCE_DIR} ${DIR})
        message(STATUS "clang-format: ${DIR_SAFE}")
        string(REPLACE "/" "_" DIR_SAFE ${DIR_SAFE})

        file(GLOB_RECURSE SRC_FILES "${DIR}/*.cpp" "${DIR}/*.hpp" "${DIR}/*.mm" "${DIR}/*.m")
        add_custom_target(
                clang-format_${DIR_SAFE}
                COMMAND ${CLANG_FORMAT_CMD}
                -style=file
                -i
                ${SRC_FILES}
        )
        add_dependencies(clang-format clang-format_${DIR_SAFE})
        set_target_properties(clang-format_${DIR_SAFE} PROPERTIES FOLDER clang-format)
    endif ()

endmacro(CLANG_FORMAT)