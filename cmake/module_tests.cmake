function(ADD_TEST_MODULE MODULE_ON_TEST)

    # Determine test exe name
    get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    string(REPLACE " " "_" MODULE_NAME ${MODULE_NAME})
    set(MODULE_NAME ${MODULE_NAME}_tests)

    message(STATUS "TESTS: Adding test module ${MODULE_NAME}")

    # Find all source files
    file(GLOB_RECURSE SRC_FILES
            RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")


    # Fixtures Dir
    set(FIXTURES_DIR ${CMAKE_SOURCE_DIR}/test/fixtures)

    # Common test files (driver)
    set(COMMON_TEST_DIR ${CMAKE_SOURCE_DIR}/test/common)

    # Create the test executable
    add_executable(${MODULE_NAME}
            ${COMMON_TEST_DIR}/test.hpp
            ${COMMON_TEST_DIR}/main.cpp

            ${SRC_FILES}
            )
    target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${COMMON_TEST_DIR})
    target_link_libraries(${MODULE_NAME} PUBLIC gtest gmock ${MODULE_ON_TEST})
    target_compile_definitions(${MODULE_NAME} PRIVATE FIXTURES_DIR=\"${FIXTURES_DIR}\")

    # Add a CTest entry
    add_test(NAME ${MODULE_NAME} COMMAND ${MODULE_NAME})

    # Copy any resources to the binary dir
    file(GLOB files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.glsl" "resources/*.*" "resources/**/*.*")
    foreach (file ${files})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
    endforeach ()

    target_link_libraries(${MODULE_NAME} INTERFACE frck-general-compile-options)

    # We like clean test code as well
    clang_tidy(${MODULE_NAME})

    # Add clang-format target
    clang_format(${CMAKE_CURRENT_SOURCE_DIR})

endfunction(ADD_TEST_MODULE)