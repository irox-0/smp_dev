// TableTest.cpp
#include <gtest/gtest.h>
#include "ui/widgets/Table.hpp"
#include "utils/Console.hpp"

using namespace StockMarketSimulator;

class TableTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Настраиваем тестовую таблицу
        testHeaders = {"ID", "Name", "Price", "Change"};
        testData = {
            {"1", "Tech Corp", "100.00", "+5.2%"},
            {"2", "Energy Inc", "50.50", "-1.3%"},
            {"3", "Finance Ltd", "75.25", "+0.8%"}
        };

        table = std::make_unique<Table>(0, 0, testHeaders,
                                       std::vector<int>{5, 15, 10, 10});

        for (const auto& row : testData) {
            table->addRow(row);
        }
    }

    std::vector<std::string> testHeaders;
    std::vector<std::vector<std::string>> testData;
    std::unique_ptr<Table> table;
};

TEST_F(TableTest, DefaultConstructorTest) {
    Table defaultTable;

    ASSERT_EQ(defaultTable.getX(), 0);
    ASSERT_EQ(defaultTable.getY(), 0);
    ASSERT_TRUE(defaultTable.getHasHeader());
    ASSERT_EQ(defaultTable.getHeaderFg(), TextColor::White);
    ASSERT_EQ(defaultTable.getHeaderBg(), TextColor::Blue);
    ASSERT_EQ(defaultTable.getBodyFg(), TextColor::Default);
    ASSERT_EQ(defaultTable.getBodyBg(), TextColor::Default);
    ASSERT_TRUE(defaultTable.isVisible());
    ASSERT_TRUE(defaultTable.getHeaders().empty());
    ASSERT_TRUE(defaultTable.getData().empty());
    ASSERT_TRUE(defaultTable.getColumnWidths().empty());
}

TEST_F(TableTest, ParameterizedConstructorTest) {
    Table paramTable(5, 10, testHeaders, std::vector<int>{5, 15, 10, 10});

    ASSERT_EQ(paramTable.getX(), 5);
    ASSERT_EQ(paramTable.getY(), 10);
    ASSERT_TRUE(paramTable.getHasHeader());
    ASSERT_EQ(paramTable.getHeaders(), testHeaders);
    ASSERT_EQ(paramTable.getColumnWidths(), (std::vector<int>{5, 15, 10, 10}));
}

TEST_F(TableTest, SettersTest) {
    Table testTable;

    testTable.setPosition(5, 10);
    ASSERT_EQ(testTable.getX(), 5);
    ASSERT_EQ(testTable.getY(), 10);

    testTable.setHeaders(testHeaders);
    ASSERT_EQ(testTable.getHeaders(), testHeaders);

    std::vector<int> widths{10, 20, 30, 40};
    testTable.setColumnWidths(widths);
    ASSERT_EQ(testTable.getColumnWidths(), widths);

    testTable.setHeaderColors(TextColor::Red, TextColor::Green);
    ASSERT_EQ(testTable.getHeaderFg(), TextColor::Red);
    ASSERT_EQ(testTable.getHeaderBg(), TextColor::Green);

    testTable.setBodyColors(TextColor::Blue, TextColor::Yellow);
    ASSERT_EQ(testTable.getBodyFg(), TextColor::Blue);
    ASSERT_EQ(testTable.getBodyBg(), TextColor::Yellow);

    testTable.setHasHeader(false);
    ASSERT_FALSE(testTable.getHasHeader());

    testTable.setVisible(false);
    ASSERT_FALSE(testTable.isVisible());
}

TEST_F(TableTest, DataManagementTest) {
    ASSERT_EQ(table->getData(), testData);

    std::vector<std::string> newRow{"4", "Consumer Co", "25.75", "+2.1%"};
    table->addRow(newRow);

    std::vector<std::vector<std::string>> expectedData = testData;
    expectedData.push_back(newRow);
    ASSERT_EQ(table->getData(), expectedData);

    table->removeRow(1);

    expectedData.erase(expectedData.begin() + 1);
    ASSERT_EQ(table->getData(), expectedData);

    std::vector<std::string> updatedRow{"3", "Finance Ltd", "80.00", "+6.3%"};
    table->updateRow(1, updatedRow);

    expectedData[1] = updatedRow;
    ASSERT_EQ(table->getData(), expectedData);

    table->clearData();
    ASSERT_TRUE(table->getData().empty());
}

TEST_F(TableTest, SortTest) {
    table->sortByColumn(2, true);

    std::vector<std::vector<std::string>> expectedSortedData = {
        {"2", "Energy Inc", "50.50", "-1.3%"},
        {"3", "Finance Ltd", "75.25", "+0.8%"},
        {"1", "Tech Corp", "100.00", "+5.2%"}
    };

    ASSERT_EQ(table->getData(), expectedSortedData);

    table->sortByColumn(2, false);

    std::vector<std::vector<std::string>> expectedReverseSortedData = {
        {"1", "Tech Corp", "100.00", "+5.2%"},
        {"3", "Finance Ltd", "75.25", "+0.8%"},
        {"2", "Energy Inc", "50.50", "-1.3%"}
    };

    ASSERT_EQ(table->getData(), expectedReverseSortedData);

    table->setSortFunction([](const std::vector<std::string>& a,
                           const std::vector<std::string>& b,
                           int columnIndex) {
        auto extractValue = [](const std::string& s) {
            double value = 0.0;
            try {
                size_t pos = 0;
                while (pos < s.length() && !isdigit(s[pos]) && s[pos] != '-' && s[pos] != '+') {
                    pos++;
                }

                value = std::stod(s.substr(pos));
            } catch (...) {
            }
            return value;
        };

        double valueA = extractValue(a[columnIndex]);
        double valueB = extractValue(b[columnIndex]);

        return valueA < valueB;
    });

    table->sortByColumn(3, true);

    std::vector<std::vector<std::string>> expectedCustomSortedData = {
        {"2", "Energy Inc", "50.50", "-1.3%"},
        {"3", "Finance Ltd", "75.25", "+0.8%"},
        {"1", "Tech Corp", "100.00", "+5.2%"}
    };

    ASSERT_EQ(table->getData(), expectedCustomSortedData);
}

TEST_F(TableTest, CalculationsTest) {
    int expectedHeight = 3 + 1 + 1;
    ASSERT_EQ(table->calculateTableHeight(), expectedHeight);

    int expectedWidth = 1 + 5 + 1 + 15 + 1 + 10 + 1 + 10 + 1;
    ASSERT_EQ(table->calculateTableWidth(), expectedWidth);
}

TEST_F(TableTest, DrawTest) {
    ASSERT_NO_THROW(table->draw());

    table->setVisible(false);
    ASSERT_NO_THROW(table->draw());

    Table emptyTable;
    ASSERT_NO_THROW(emptyTable.draw());
}

TEST_F(TableTest, EdgeCasesTest) {
    std::vector<std::string> shortRow{"5", "Short"};
    table->addRow(shortRow);

    std::vector<std::string> expectedRow{"5", "Short", "", ""};
    ASSERT_EQ(table->getData().back(), expectedRow);

    std::vector<std::string> longRow{"1", "Too", "Long", "Row", "Extra"};
    table->updateRow(0, longRow);

    std::vector<std::string> expectedUpdatedRow{"1", "Too", "Long", "Row"};
    ASSERT_EQ(table->getData()[0], expectedUpdatedRow);

    size_t initialSize = table->getData().size();
    table->removeRow(100);
    ASSERT_EQ(table->getData().size(), initialSize);

    ASSERT_NO_THROW(table->sortByColumn(100));
}