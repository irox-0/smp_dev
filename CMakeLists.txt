cmake_minimum_required(VERSION 3.14)
project(StockMarketSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${SOURCE_DIR})

file(GLOB_RECURSE UTILS_SOURCES
        "${SOURCE_DIR}/utils/*.cpp"
)

file(GLOB_RECURSE CORE_SOURCES
        "${SOURCE_DIR}/core/*.cpp"
)

file(GLOB_RECURSE MODELS_SOURCES
        "${SOURCE_DIR}/models/*.cpp"
)

file(GLOB_RECURSE UI_SOURCES
        "${SOURCE_DIR}/ui/*.cpp"
)

file(GLOB_RECURSE SERVICES_SOURCES
        "${SOURCE_DIR}/services/*.cpp"
)

add_library(stock_market_utils
        ${UTILS_SOURCES}
        ${CORE_SOURCES}
        ${MODELS_SOURCES}
        ${UI_SOURCES}
        ${SERVICES_SOURCES}
)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/utils/*.cpp"
)

enable_testing()
include(GoogleTest)

add_executable(utils_tests ${TEST_SOURCES}
        tests/utils/ConsoleTest.cpp
)

target_link_libraries(utils_tests
        PRIVATE
        stock_market_utils
        gtest
        gtest_main
)

gtest_discover_tests(utils_tests)