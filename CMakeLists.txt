cmake_minimum_required (VERSION 3.16)

file(READ "version" VERSION_OUTPUT)
string(STRIP ${VERSION_OUTPUT} PACKAGE_VERSION)

project(simulator-intel-8080
    DESCRIPTION "Intel 8080 Simulator"
    VERSION ${PACKAGE_VERSION}
    LANGUAGES CXX
)

if(NOT CMAKE_CXX_STANDARD) 
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(LINUX FALSE)
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(ENABLE_TESTING_DEFAULT OFF)
option(ENABLE_TESTING "Include unittest related targtes" 
    ${ENABLE_TESTING_DEFAULT}
)

if(MSVC)
    add_compile_options(/WX /W4 /EHsc)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wimplicit-fallthrough -Werror)
endif()

message(STATUS "Build Configuration")
message(STATUS "Enable testing:" ${ENABLE_TESTING})
message(STATUS "CMake Generator:" ${CMAKE_GENERATOR})
message(STATUS "C++ Flags:" ${CMAKE_CXX_FLAGS})
message(STATUS "List of compile features:" ${CMAKE_CXX_COMPILE_FEATURES})
message(STATUS "Toolchain file:" ${CMAKE_TOOLCHAIN_FILE})

find_package(spdlog REQUIRED)
find_package(CLI11 REQUIRED)
find_package(SystemCLanguage REQUIRED CONFIG)

if(ENABLE_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)
    enable_testing()
    add_subdirectory(tests)
endif()

set(headers
    alu.hpp
    memory.hpp
    log.hpp
    utils.hpp
)

set(sources
    alu.cpp
    memory.cpp
    main.cpp
    log.cpp
    utils.cpp
)

if(APPLE)
    set(headers ${headers} oslogger.hpp)
endif()

list(TRANSFORM headers PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/include/")
list(TRANSFORM sources PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp.in version.hpp)

source_group("include" FILES ${headers})
source_group("src" FILES ${sources})

set(conan_libs spdlog::spdlog CLI11::CLI11 SystemC::systemc)

if(LINUX)
    set(conan_libs ${conan_libs} stdc++fs)
endif()

add_executable (${PROJECT_NAME} ${sources} ${headers})

target_link_libraries(${PROJECT_NAME} ${conan_libs})

target_include_directories(${PROJECT_NAME} PRIVATE 
    "${PROJECT_SOURCE_DIR}/include"
    ${CMAKE_CURRENT_BINARY_DIR}
)

if (APPLE)
    # It's OK that __sanitizer_start_switch_fiber, and
    # __sanitizer_finish_switch_fiber are undefined symbols.
    set_target_properties (${PROJECT_NAME} PROPERTIES LINK_FLAGS
    -Wl,-U,___sanitizer_start_switch_fiber,-U,___sanitizer_finish_switch_fiber)
endif(APPLE)
