#include <gtest/gtest.h>
#include "models/Stock.hpp"
#include "models/Company.hpp"

using namespace StockMarketSimulator;

class StockTest : public ::testing::Test {
protected:
    void SetUp() override {
        testCompany = std::make_shared<Company>(
            "Test Company", "TEST",
            "A company for testing", Sector::Technology,
            100.0, 0.5, DividendPolicy(2.0, 4)
        );
        testStock = std::make_unique<Stock>(testCompany, 100.0);
    }

    std::shared_ptr<Company> testCompany;
    std::unique_ptr<Stock> testStock;
};

TEST_F(StockTest, InitializationTest) {
    ASSERT_EQ(testStock->getCurrentPrice(), 100.0);
    ASSERT_EQ(testStock->getOpenPrice(), 100.0);
    ASSERT_EQ(testStock->getPreviousClosePrice(), 100.0);
    ASSERT_EQ(testStock->getHighestPrice(), 100.0);
    ASSERT_EQ(testStock->getLowestPrice(), 100.0);
    ASSERT_EQ(testStock->getDayChangeAmount(), 0.0);
    ASSERT_EQ(testStock->getDayChangePercent(), 0.0);
    ASSERT_EQ(testStock->getPriceHistoryLength(), 1);
}

TEST_F(StockTest, UpdatePriceTest) {
    testStock->updatePrice(110.0);

    ASSERT_EQ(testStock->getCurrentPrice(), 110.0);
    ASSERT_EQ(testStock->getHighestPrice(), 110.0);
    ASSERT_EQ(testStock->getLowestPrice(), 100.0);
    ASSERT_EQ(testStock->getDayChangeAmount(), 10.0);
    ASSERT_NEAR(testStock->getDayChangePercent(), 10.0, 0.001);
    ASSERT_EQ(testStock->getPriceHistoryLength(), 2);

    testStock->updatePrice(90.0);

    ASSERT_EQ(testStock->getCurrentPrice(), 90.0);
    ASSERT_EQ(testStock->getHighestPrice(), 110.0);
    ASSERT_EQ(testStock->getLowestPrice(), 90.0);
    ASSERT_EQ(testStock->getDayChangeAmount(), -10.0);
    ASSERT_NEAR(testStock->getDayChangePercent(), -10.0, 0.001);
    ASSERT_EQ(testStock->getPriceHistoryLength(), 3);
}

TEST_F(StockTest, DayCloseOpenTest) {
    testStock->updatePrice(110.0);
    testStock->closeDay();

    ASSERT_EQ(testStock->getPreviousClosePrice(), 110.0);

    testStock->openDay();

    ASSERT_EQ(testStock->getOpenPrice(), 110.0);
    ASSERT_EQ(testStock->getDayChangeAmount(), 0.0);
    ASSERT_EQ(testStock->getDayChangePercent(), 0.0);

    testStock->updatePrice(120.0);

    ASSERT_EQ(testStock->getDayChangeAmount(), 10.0);
    ASSERT_NEAR(testStock->getDayChangePercent(), 9.09, 0.01);
}

TEST_F(StockTest, MarketInfluenceTest) {
    testStock->setMarketInfluence(0.8);
    testStock->updatePriceWithMarketInfluence(0.05);

    ASSERT_NEAR(testStock->getCurrentPrice(), 104.0, 0.001);
}

TEST_F(StockTest, SectorInfluenceTest) {
    testStock->setSectorInfluence(0.6);
    testStock->updatePriceWithSectorInfluence(0.08);

    ASSERT_NEAR(testStock->getCurrentPrice(), 104.8, 0.001);
}

TEST_F(StockTest, NewsImpactTest) {
    testStock->updatePriceWithNewsImpact(0.15);

    ASSERT_NEAR(testStock->getCurrentPrice(), 115.0, 0.001);
}

TEST_F(StockTest, JsonSerializationTest) {
    testStock->updatePrice(120.0);
    testStock->closeDay();
    testStock->updatePrice(125.0);

    nlohmann::json json = testStock->toJson();

    ASSERT_EQ(json["current_price"], 125.0);
    ASSERT_EQ(json["previous_close_price"], 120.0);
    ASSERT_EQ(json["highest_price"], 125.0);
    ASSERT_EQ(json["lowest_price"], 100.0);

    Stock newStock = Stock::fromJson(json, testCompany);

    ASSERT_EQ(newStock.getCurrentPrice(), 125.0);
    ASSERT_EQ(newStock.getPreviousClosePrice(), 120.0);
    ASSERT_EQ(newStock.getHighestPrice(), 125.0);
    ASSERT_EQ(newStock.getLowestPrice(), 100.0);
}

TEST_F(StockTest, PreventZeroPriceTest) {
    testStock->updatePrice(-10.0);

    ASSERT_EQ(testStock->getCurrentPrice(), 0.01);
}