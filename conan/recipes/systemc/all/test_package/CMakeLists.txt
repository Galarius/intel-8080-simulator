cmake_minimum_required(VERSION 3.15)
project(test_package CXX)

find_package(SystemCLanguage REQUIRED CONFIG)

add_executable(${PROJECT_NAME} test_package.cpp)
target_link_libraries(${PROJECT_NAME} SystemC::systemc)

if (APPLE)
    # It's OK that _sc_main, __sanitizer_start_switch_fiber, and
    # __sanitizer_finish_switch_fiber are undefined symbols.
    set_target_properties (test_package PROPERTIES LINK_FLAGS
    -Wl,-U,___sanitizer_start_switch_fiber,-U,___sanitizer_finish_switch_fiber)
endif(APPLE)