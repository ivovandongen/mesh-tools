cmake_minimum_required(VERSION 3.22)
project(meshtools)

set(CMAKE_CXX_STANDARD 17)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
    set(MESHTOOLS_SUB_BUILD OFF)
else ()
    message(STATUS "Sub-project build. Root SRC directory ${CMAKE_SOURCE_DIR} - Meshtools dir ${PROJECT_SOURCE_DIR}")
    set(MESHTOOLS_SUB_BUILD ON)
endif ()

# Options
option(ENABLE_CCACHE "Speed up incremental rebuilds via ccache" ON)
option(ENABLE_CLANG_FORMAT "Check sources with clang-format" ON)
option(ENABLE_CLANG_TIDY "Check sources with clang-tidy" ON)
option(ENABLE_CLANG_TIDY_FIX "Fix sources with clang-tidy (depends on ENABLE_CLANG_TIDY)" OFF)
option(ENABLE_GIT_SUBMODULE_UPDATES "Automatically update submodules" OFF)
option(ENABLE_TESTS "Enable tests" ON)
option(ENABLE_ADDRESS_SANITIZER "Enable Asan https://clang.llvm.org/docs/AddressSanitizer.html" OFF)

# CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

# Load CMake modules
include(${PROJECT_SOURCE_DIR}/cmake/git.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/ccache.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/clang-format.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/clang-tidy.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/include_vendor_pkg.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/include_guard.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/module.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/module_tests.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/objective_cpp.cmake)

# Sanitation
if (ENABLE_ADDRESS_SANITIZER)
    message(STATUS "Enabling Clang Address Sanitizer")
    set(ENV{ASAN_OPTIONS}, "detect_leaks=1:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:halt_on_error=0")
endif ()

# Compile options
add_library(meshtools-general-compile-options INTERFACE)
target_compile_options(
        meshtools-general-compile-options INTERFACE

        # ARC
        $<$<BOOL:${APPLE}>:-fobjc-arc>

        # Address Sanitizer options
        $<$<BOOL:${ENABLE_ADDRESS_SANITIZER}>:-fsanitize=address>
        $<$<BOOL:${ENABLE_ADDRESS_SANITIZER}>:-fno-omit-frame-pointer>
)

target_link_libraries(
        meshtools-general-compile-options INTERFACE

        # Address Sanitizer linker options
        $<$<BOOL:${ENABLE_ADDRESS_SANITIZER}>:-fsanitize=address>
)

install(TARGETS meshtools-general-compile-options EXPORT MeshtoolsTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )

# Add modules
add_subdirectory(modules)

# Always enable testing for simplicity
enable_testing()

if (NOT MESHTOOLS_SUB_BUILD)
    # Add tests
    if (ENABLE_TESTS)
        message(STATUS "Enabling tests")
        add_subdirectory(test)
    endif ()

    export(EXPORT MeshtoolsDependenciesTargets
            FILE "${CMAKE_CURRENT_BINARY_DIR}/Meshtools/MeshtoolsDependenciesTargets.cmake"
            NAMESPACE Meshtools::Dependencies
            )

    export(EXPORT MeshtoolsTargets
            FILE "${CMAKE_CURRENT_BINARY_DIR}/Meshtools/MeshtoolsTargets.cmake"
            NAMESPACE Meshtools::
            )
endif ()