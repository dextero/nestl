cmake_minimum_required(VERSION 3.10)
project(nestl VERSION 0.1 LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

install(DIRECTORY include
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")

add_library(nestl INTERFACE)
target_include_directories(nestl INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:include>)
target_compile_features(nestl INTERFACE cxx_std_17)

add_executable(nestl_test test/main.cpp)
target_sources(nestl_test PRIVATE
               test/result.cpp
               test/variant.cpp
               test/vector.cpp)
target_link_libraries(nestl_test PRIVATE nestl)
target_compile_options(nestl_test PRIVATE
                       $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Weverything>)

enable_testing()
add_test(NAME nestl COMMAND $<TARGET_FILE:nestl_test> DEPENDS nestl_test)

option(NESTL_STATIC_ANALYSIS "Enable static analysis tools" ON)
if(NESTL_STATIC_ANALYSIS)
    find_program(CLANG_TIDY NAMES clang-tidy REQUIRED)
    find_program(CPPCHECK NAMES cppcheck REQUIRED)
    find_program(CPPLINT NAMES cpplint REQUIRED)
    find_program(INCLUDE_WHAT_YOU_USE NAMES include-what-you-use iwyu REQUIRED)

    set_target_properties(nestl_test PROPERTIES
                          CXX_CLANG_TIDY "${CLANG_TIDY};--checks=*,-fuchsia-default-arguments"
                          CXX_CPPCHECK "${CPPCHECK};--force;--quiet"
                          CXX_CPPLINT "${CPPLINT}"
                          CXX_INCLUDE_WHAT_YOU_USE "${INCLUDE_WHAT_YOU_USE}")
endif()
