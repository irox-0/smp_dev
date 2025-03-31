#include <gtest/gtest.h>
#include "../../src/utils/Console.hpp"
#include <sstream>

using namespace StockMarketSimulator;

TEST(ConsoleTest, InitializeTest) {
    ASSERT_NO_THROW(Console::initialize());
}

TEST(ConsoleTest, GetSizeTest) {
    auto size = Console::getSize();
    ASSERT_GT(size.first, 0);
    ASSERT_GT(size.second, 0);
}

TEST(ConsoleTest, TextFormattingTest) {
    ASSERT_NO_THROW(Console::setColor(TextColor::Red));
    ASSERT_NO_THROW(Console::setColor(TextColor::Green, TextColor::Blue));
    ASSERT_NO_THROW(Console::setStyle(TextStyle::Bold));
    ASSERT_NO_THROW(Console::setStyle(TextStyle::Underline));
    ASSERT_NO_THROW(Console::resetAttributes());
    ASSERT_NO_THROW(Console::print("Test"));
    ASSERT_NO_THROW(Console::println("Test"));
}

TEST(ConsoleTest, DrawingMethodsTest) {
    ASSERT_NO_THROW(Console::clear());
    ASSERT_NO_THROW(Console::setCursorPosition(0, 0));
    ASSERT_NO_THROW(Console::drawBox(0, 0, 10, 5));
    ASSERT_NO_THROW(Console::drawHorizontalLine(0, 0, 10));
    ASSERT_NO_THROW(Console::drawVerticalLine(0, 0, 5));

    std::vector<std::vector<std::string>> tableData = {
        {"Header1", "Header2"},
        {"Data1", "Data2"}
    };
    std::vector<int> columnWidths = {10, 10};
    ASSERT_NO_THROW(Console::drawTable(0, 0, tableData, columnWidths));

    std::vector<double> chartData = {1.0, 2.0, 3.0, 2.5, 4.0};
    ASSERT_NO_THROW(Console::drawChart(0, 0, 20, 10, chartData, 0.0, 5.0));

    ASSERT_NO_THROW(Console::drawProgressBar(0, 0, 20, 0.5));
}

TEST(ConsoleTest, DrawTableEdgeCasesTest) {
    std::vector<std::vector<std::string>> emptyData;
    std::vector<int> columnWidths = {10, 10};
    ASSERT_NO_THROW(Console::drawTable(0, 0, emptyData, columnWidths));

    std::vector<std::vector<std::string>> tableData = {{"Test"}};
    std::vector<int> emptyWidths;
    ASSERT_NO_THROW(Console::drawTable(0, 0, tableData, emptyWidths));

    std::vector<std::vector<std::string>> longTextData = {
        {"Header with very long text", "Header2"},
        {"Data with text longer than column width", "Data2"}
    };
    std::vector<int> narrowWidths = {10, 10};
    ASSERT_NO_THROW(Console::drawTable(0, 0, longTextData, narrowWidths));
}

TEST(ConsoleTest, ChartSpecialCasesTest) {
    std::vector<double> emptyData;
    ASSERT_NO_THROW(Console::drawChart(0, 0, 20, 10, emptyData, 0.0, 1.0));
    std::vector<double> sameValueData = {5.0, 5.0, 5.0};
    ASSERT_NO_THROW(Console::drawChart(0, 0, 20, 10, sameValueData, 5.0, 5.0));

    std::vector<double> normalData = {1.0, 2.0, 3.0};
    ASSERT_NO_THROW(Console::drawChart(0, 0, 2, 2, normalData, 0.0, 5.0));

    std::vector<double> manyDataPoints;
    for (int i = 0; i < 100; i++) {
        manyDataPoints.push_back(static_cast<double>(i % 10));
    }
    ASSERT_NO_THROW(Console::drawChart(0, 0, 20, 10, manyDataPoints, 0.0, 10.0));
}

TEST(ConsoleTest, ProgressBarEdgeCasesTest) {
    ASSERT_NO_THROW(Console::drawProgressBar(0, 0, 0, 0.5));
    ASSERT_NO_THROW(Console::drawProgressBar(0, 0, 20, -0.5));
    ASSERT_NO_THROW(Console::drawProgressBar(0, 0, 20, 1.5));
}

TEST(ConsoleTest, MenuEmptyOptionsTest) {
    std::vector<std::string> emptyOptions;
    int result = Console::showMenu(0, 0, emptyOptions);
    ASSERT_EQ(result, -1);
}

TEST(ConsoleTest, UtilityMethodsTest) {
    ASSERT_NO_THROW(Console::sleep(1));
}
