cmake_minimum_required(VERSION 3.22)
project(mesh_tools)

set(CMAKE_CXX_STANDARD 17)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Options

option(ENABLE_CCACHE "Speed up incremental rebuilds via ccache" ON)
option(ENABLE_CLANG_FORMAT "Check sources with clang-format" ON)
option(ENABLE_CLANG_TIDY "Check sources with clang-tidy" ON)
option(ENABLE_CLANG_TIDY_FIX "Fix sources with clang-tidy (depends on ENABLE_CLANG_TIDY)" OFF)
option(ENABLE_GIT_SUBMODULE_UPDATES "Automatically update submodules" OFF)
option(ENABLE_TESTS "Enable tests" ON)
option(ENABLE_ADDRESS_SANITIZER "Enable Asan https://clang.llvm.org/docs/AddressSanitizer.html" OFF)

# CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Load CMake modules
include(${CMAKE_MODULE_PATH}/git.cmake)
include(${CMAKE_MODULE_PATH}/ccache.cmake)
include(${CMAKE_MODULE_PATH}/clang-format.cmake)
include(${CMAKE_MODULE_PATH}/clang-tidy.cmake)
include(${CMAKE_MODULE_PATH}/include_vendor_pkg.cmake)
include(${CMAKE_MODULE_PATH}/include_guard.cmake)
include(${CMAKE_MODULE_PATH}/module.cmake)
include(${CMAKE_MODULE_PATH}/module_tests.cmake)
include(${CMAKE_MODULE_PATH}/objective_cpp.cmake)

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

# Add modules
add_subdirectory(modules)

# Add tests
if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()
