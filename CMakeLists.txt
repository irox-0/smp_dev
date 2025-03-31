cmake_minimum_required(VERSION 3.14)
project(StockMarketSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${SOURCE_DIR})

include(FetchContent)
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)


file(GLOB_RECURSE SMP_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(smp ${SMP_SOURCES})
target_link_libraries(smp PRIVATE nlohmann_json::nlohmann_json)

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

target_link_libraries(stock_market_utils PRIVATE nlohmann_json::nlohmann_json)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

add_executable(utils_tests
        tests/utils/RandomTest.cpp
        tests/utils/ConsoleTest.cpp
        tests/utils/FileIOTest.cpp
        tests/utils/DateTest.cpp
#        tests/models/CompanyTest.cpp
#        tests/models/StockTest.cpp
#        tests/models/DividedPolicyTest.cpp
#        tests/core/MarketTest.cpp
#        tests/models/LoanTest.cpp
#        tests/models/TransactionTest.cpp
#        tests/models/PortfolioTest.cpp
#        tests/core/PlayerTest.cpp
#        tests/models/NewsTest.cpp
#        tests/services/NewsServiceTest.cpp
#        tests/services/PriceServiceTest.cpp
#        tests/services/SaveServiceTest.cpp
#        tests/ui/widgets/ChartTest.cpp
#        tests/ui/widgets/MenuTest.cpp
#        tests/ui/widgets/TableTest.cpp
#        tests/ui/ScreenTest.cpp
#        tests/ui/screens/MainScreenTest.cpp
#        tests/ui/screens/NewsScreenTest.cpp
#        tests/ui/screens/MarketScreenTest.cpp
#        tests/ui/screens/FinancialScreenTest.cpp
#        tests/ui/screens/CompanyScreenTest.cpp
#        tests/ui/screens/MarketCompanyIntegrationTest.cpp
)

target_link_libraries(utils_tests
        PRIVATE
        stock_market_utils
        gtest
        gtest_main
        nlohmann_json::nlohmann_json
)

gtest_discover_tests(utils_tests)