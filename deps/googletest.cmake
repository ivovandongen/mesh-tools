include_guard_x()

set(GTEST_DIR ${CMAKE_SOURCE_DIR}/deps/googletest/googletest)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
if (CMAKE_USE_PTHREADS_INIT)
    set(GTEST_HAS_PTHREAD ON)
endif()

add_library(gtest STATIC
        ${GTEST_DIR}/src/gtest.cc
        ${GTEST_DIR}/src/gtest-death-test.cc
        ${GTEST_DIR}/src/gtest-filepath.cc
        ${GTEST_DIR}/src/gtest-port.cc
        ${GTEST_DIR}/src/gtest-printers.cc
        ${GTEST_DIR}/src/gtest-test-part.cc
        ${GTEST_DIR}/src/gtest-typed-test.cc
        )

target_include_directories(gtest
        PRIVATE ${GTEST_DIR}/include
        PRIVATE ${GTEST_DIR}
        )

target_include_directories(gtest
        SYSTEM INTERFACE ${GTEST_DIR}/include
        )

target_link_libraries(gtest PUBLIC Threads::Threads)


set(GMOCK_DIR ${CMAKE_SOURCE_DIR}/deps/googletest/googlemock)

add_library(gmock STATIC
        ${GMOCK_DIR}/src/gmock.cc
        ${GMOCK_DIR}/src/gmock-all.cc
        ${GMOCK_DIR}/src/gmock-cardinalities.cc
        ${GMOCK_DIR}/src/gmock-internal-utils.cc
        ${GMOCK_DIR}/src/gmock-matchers.cc
        ${GMOCK_DIR}/src/gmock-spec-builders.cc
        )

target_include_directories(gmock
        PRIVATE ${GMOCK_DIR}/include
        PRIVATE ${GMOCK_DIR}
        )

target_include_directories(gmock
        SYSTEM INTERFACE ${GMOCK_DIR}/include
        )

target_link_libraries(gmock PUBLIC gtest)