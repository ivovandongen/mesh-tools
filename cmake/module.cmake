macro(ADD_MODULE MODULE_NAME)

    message(STATUS "modules: adding module ${MODULE_NAME}")

    file(GLOB_RECURSE HEADER_FILES
            RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

    file(GLOB_RECURSE SRC_FILES
            RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*.mm"
            )

    set(${MODULE_NAME}_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/)

    if (src/main.cpp IN_LIST SRC_FILES)
        # Executable
        message(STATUS "modules: adding executable ${MODULE_NAME}")
        add_executable(${MODULE_NAME}
                ${HEADER_FILES}
                ${SRC_FILES}
                )

        target_include_directories(${MODULE_NAME} PRIVATE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
                )
    elseif (NOT "${SRC_FILES}" STREQUAL "")
        # Static library
        add_library(${MODULE_NAME}
                ${HEADER_FILES}

                ${SRC_FILES}
                )

        target_include_directories(${MODULE_NAME} PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
                )

        target_link_libraries(${MODULE_NAME} PUBLIC meshtools-general-compile-options)

        clang_tidy(${MODULE_NAME})
    else ()
        # Interface library
        add_library(${MODULE_NAME} INTERFACE)

        target_include_directories(${MODULE_NAME} INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
                )

        target_link_libraries(${MODULE_NAME} INTERFACE meshtools-general-compile-options)
    endif ()

    # Add clang-format target
    clang_format(${CMAKE_CURRENT_SOURCE_DIR})

    # Organize our build
    set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules)

endmacro(ADD_MODULE)