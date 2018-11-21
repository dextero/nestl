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
               test/variant.cpp)
target_link_libraries(nestl_test PRIVATE nestl)

enable_testing()
add_test(NAME nestl COMMAND $<TARGET_FILE:nestl_test> DEPENDS nestl_test)
