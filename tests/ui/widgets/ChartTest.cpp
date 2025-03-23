#include <gtest/gtest.h>
#include "ui/widgets/Chart.hpp"
#include "utils/Console.hpp"

using namespace StockMarketSimulator;

class ChartTest : public ::testing::Test {
protected:
    void SetUp() override {
        testData = {1.0, 2.5, 1.5, 3.0, 2.0, 4.0, 3.5};
        xLabels = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    }

    std::vector<double> testData;
    std::vector<std::string> xLabels;
};

TEST_F(ChartTest, DefaultConstructorTest) {
    Chart chart;

    ASSERT_EQ(chart.getX(), 0);
    ASSERT_EQ(chart.getY(), 0);
    ASSERT_EQ(chart.getWidth(), 40);
    ASSERT_EQ(chart.getHeight(), 10);
    ASSERT_EQ(chart.getTitle(), "");
    ASSERT_EQ(chart.getColor(), TextColor::Green);
    ASSERT_TRUE(chart.getAutoScale());
    ASSERT_TRUE(chart.getData().empty());
    ASSERT_TRUE(chart.getXLabels().empty());
    ASSERT_EQ(chart.getYLabel(), "");
}

TEST_F(ChartTest, ParameterizedConstructorTest) {
    Chart chart(5, 5, 50, 15, "Test Chart");

    ASSERT_EQ(chart.getX(), 5);
    ASSERT_EQ(chart.getY(), 5);
    ASSERT_EQ(chart.getWidth(), 50);
    ASSERT_EQ(chart.getHeight(), 15);
    ASSERT_EQ(chart.getTitle(), "Test Chart");
    ASSERT_EQ(chart.getColor(), TextColor::Green);
    ASSERT_TRUE(chart.getAutoScale());
    ASSERT_TRUE(chart.getData().empty());
    ASSERT_TRUE(chart.getXLabels().empty());
    ASSERT_EQ(chart.getYLabel(), "");
}

TEST_F(ChartTest, SettersTest) {
    Chart chart;

    chart.setPosition(10, 10);
    chart.setSize(60, 20);
    chart.setTitle("Stock Prices");
    chart.setColor(TextColor::Blue);
    chart.setAutoScale(false);
    chart.setData(testData);
    chart.setXLabels(xLabels);
    chart.setYLabel("Price");

    ASSERT_EQ(chart.getX(), 10);
    ASSERT_EQ(chart.getY(), 10);
    ASSERT_EQ(chart.getWidth(), 60);
    ASSERT_EQ(chart.getHeight(), 20);
    ASSERT_EQ(chart.getTitle(), "Stock Prices");
    ASSERT_EQ(chart.getColor(), TextColor::Blue);
    ASSERT_FALSE(chart.getAutoScale());
    ASSERT_EQ(chart.getData(), testData);
    ASSERT_EQ(chart.getXLabels(), xLabels);
    ASSERT_EQ(chart.getYLabel(), "Price");
}

TEST_F(ChartTest, DataManipulationTest) {
    Chart chart;

    chart.setData(testData);
    ASSERT_EQ(chart.getData(), testData);

    chart.addDataPoint(5.0);
    std::vector<double> expectedData = testData;
    expectedData.push_back(5.0);
    ASSERT_EQ(chart.getData(), expectedData);

    chart.clearData();
    ASSERT_TRUE(chart.getData().empty());
}

TEST_F(ChartTest, InvalidSizeTest) {
    Chart chart;

    int initialWidth = chart.getWidth();
    int initialHeight = chart.getHeight();

    chart.setSize(-10, -5);

    ASSERT_EQ(chart.getWidth(), initialWidth);
    ASSERT_EQ(chart.getHeight(), initialHeight);
}

TEST_F(ChartTest, AutoScaleTest) {
    Chart chart;
    chart.setData({1.0, 3.0, 2.0, 5.0});

    chart.setAutoScale(false);
    chart.setData({10.0, 30.0, 20.0, 50.0});

    ASSERT_NO_THROW(chart.draw());
}

TEST_F(ChartTest, DrawTest) {
    Chart chart(0, 0, 40, 15, "Stock Prices");
    chart.setData(testData);
    chart.setXLabels(xLabels);
    chart.setYLabel("Price");

    ASSERT_NO_THROW(chart.draw());
}

TEST_F(ChartTest, EmptyDataTest) {
    Chart chart(0, 0, 40, 15, "Empty Chart");

    ASSERT_NO_THROW(chart.draw());
}

TEST_F(ChartTest, LongTitleAndLabelsTest) {
    Chart chart(0, 0, 20, 10, "This is a very long title that should be truncated");

    std::vector<std::string> longLabels;
    for (int i = 0; i < 10; i++) {
        longLabels.push_back("LongLabel" + std::to_string(i));
    }

    chart.setXLabels(longLabels);
    chart.setYLabel("This is a very long Y label");
    chart.setData(testData);

    ASSERT_NO_THROW(chart.draw());
}