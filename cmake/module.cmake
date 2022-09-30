include(GenerateExportHeader)

macro(ADD_MODULE MODULE_NAME_RAW)
    if (MESHTOOLS_SUB_BUILD)
        set(MODULE_NAME "meshtools-${MODULE_NAME_RAW}")
    else ()
        set(MODULE_NAME ${MODULE_NAME_RAW})
    endif ()

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
        generate_export_header(${MODULE_NAME})

        clang_tidy(${MODULE_NAME})
        add_library(Meshtools::${MODULE_NAME_RAW} ALIAS ${MODULE_NAME})
    else ()
        # Interface library
        add_library(${MODULE_NAME} INTERFACE)

        target_include_directories(${MODULE_NAME} INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
                )

        target_link_libraries(${MODULE_NAME} INTERFACE meshtools-general-compile-options)
        generate_export_header(${MODULE_NAME})
        add_library(Meshtools::${MODULE_NAME_RAW} ALIAS ${MODULE_NAME})
    endif ()

    install(TARGETS ${MODULE_NAME} EXPORT MeshtoolsTargets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            INCLUDES DESTINATION include
            )

    # Add clang-format target
    clang_format(${CMAKE_CURRENT_SOURCE_DIR})

    # Organize our build
    set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules)

endmacro(ADD_MODULE)

macro(MESHTOOLS_MODULE_LINK_LIBRARIES)
    cmake_parse_arguments(args
            "" #options
            "TARGET" #single value
            "PRIVATE;PUBLIC" #multi value
            ${ARGN}
            )
    if (MESHTOOLS_SUB_BUILD)
        set(MODULE_NAME "meshtools-${args_TARGET}")
    else ()
        set(MODULE_NAME ${args_TARGET})
    endif ()

    target_link_libraries(${MODULE_NAME} PUBLIC ${args_PUBLIC})
    target_link_libraries(${MODULE_NAME} PRIVATE ${args_PRIVATE})
endmacro(MESHTOOLS_MODULE_LINK_LIBRARIES)