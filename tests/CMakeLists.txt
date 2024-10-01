set(TESTS_PROJECT_NAME ${PROJECT_NAME}-tests)
set(sources
    log.cpp
    utils.cpp
    alu.cpp
    cu.cpp
    memory.cpp
)
list(TRANSFORM sources PREPEND "${PROJECT_SOURCE_DIR}/src/")
set(test_sources
    main.cpp
    modules.cpp
    alu-tests.cpp
    cu-tests.cpp
    memory-tests.cpp
)
set(libs GTest::gmock spdlog::spdlog SystemC::systemc)
if(LINUX)
    set(libs ${libs} stdc++fs)
endif()

add_executable (${TESTS_PROJECT_NAME} ${sources} ${test_sources})

target_compile_definitions(${TESTS_PROJECT_NAME} PRIVATE 
    ENABLE_TESTING=1
)
target_include_directories(${TESTS_PROJECT_NAME} PRIVATE 
    "${PROJECT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries (${TESTS_PROJECT_NAME} ${libs})

if (APPLE)
    # It's OK that __sanitizer_start_switch_fiber, and
    # __sanitizer_finish_switch_fiber are undefined symbols.
    set_target_properties (${TESTS_PROJECT_NAME} PROPERTIES LINK_FLAGS
    -Wl,-U,___sanitizer_start_switch_fiber,-U,___sanitizer_finish_switch_fiber)
endif(APPLE)

gtest_discover_tests(${TESTS_PROJECT_NAME})
