#include <gtest/gtest.h>
#include <memory>
#include "../../src/models/Stock.hpp"
#include "../../src/models/Company.hpp"

using namespace StockMarketSimulator;

class StockTest : public ::testing::Test {
protected:
    std::shared_ptr<Company> company;

    void SetUp() override {
        // Create a company for testing
        company = std::make_shared<Company>(
            "TestCompany", "TEST",
            "Test company for stock tests",
            Sector::Technology,
            100.0, 0.5,
            DividendPolicy(2.0, 4)
        );
    }
};

TEST_F(StockTest, Constructor) {
    // Test default constructor
    Stock defaultStock;
    EXPECT_EQ(defaultStock.getCurrentPrice(), 0.0);
    EXPECT_EQ(defaultStock.getPriceHistory().size(), 0);
    EXPECT_EQ(defaultStock.getLastUpdateDate(), Date(1, 3, 2023)); // Default date

    // Test parameterized constructor
    double initialPrice = 100.0;
    Stock stock(company, initialPrice);
    EXPECT_EQ(stock.getCurrentPrice(), initialPrice);
    EXPECT_EQ(stock.getPriceHistory().size(), 1);
    EXPECT_EQ(stock.getPriceHistory()[0], initialPrice);
    EXPECT_EQ(stock.getHighestPrice(), initialPrice);
    EXPECT_EQ(stock.getLowestPrice(), initialPrice);
    EXPECT_EQ(stock.getPreviousClosePrice(), initialPrice);
    EXPECT_EQ(stock.getOpenPrice(), initialPrice);
}

TEST_F(StockTest, CopyConstructor) {
    // Create original stock
    Stock originalStock(company, 100.0);
    originalStock.updatePrice(110.0);

    // Use copy constructor
    Stock copiedStock(originalStock);

    // Check if copied stock has the same values
    EXPECT_EQ(copiedStock.getCurrentPrice(), originalStock.getCurrentPrice());
    EXPECT_EQ(copiedStock.getPriceHistory().size(), originalStock.getPriceHistory().size());
    EXPECT_EQ(copiedStock.getHighestPrice(), originalStock.getHighestPrice());
    EXPECT_EQ(copiedStock.getLowestPrice(), originalStock.getLowestPrice());
    EXPECT_EQ(copiedStock.getPreviousClosePrice(), originalStock.getPreviousClosePrice());
    EXPECT_EQ(copiedStock.getOpenPrice(), originalStock.getOpenPrice());
    EXPECT_EQ(copiedStock.getLastUpdateDate(), originalStock.getLastUpdateDate());
}

TEST_F(StockTest, AssignmentOperator) {
    // Create original stock
    Stock originalStock(company, 100.0);
    originalStock.updatePrice(110.0);

    // Create another stock and use assignment operator
    Stock assignedStock;
    assignedStock = originalStock;

    // Check if assigned stock has the same values
    EXPECT_EQ(assignedStock.getCurrentPrice(), originalStock.getCurrentPrice());
    EXPECT_EQ(assignedStock.getPriceHistory().size(), originalStock.getPriceHistory().size());
    EXPECT_EQ(assignedStock.getHighestPrice(), originalStock.getHighestPrice());
    EXPECT_EQ(assignedStock.getLowestPrice(), originalStock.getLowestPrice());
    EXPECT_EQ(assignedStock.getPreviousClosePrice(), originalStock.getPreviousClosePrice());
    EXPECT_EQ(assignedStock.getOpenPrice(), originalStock.getOpenPrice());
    EXPECT_EQ(assignedStock.getLastUpdateDate(), originalStock.getLastUpdateDate());
}

TEST_F(StockTest, PriceUpdate) {
    Stock stock(company, 100.0);

    // Update price
    stock.updatePrice(110.0);
    EXPECT_EQ(stock.getCurrentPrice(), 110.0);
    EXPECT_EQ(stock.getPriceHistory().size(), 2);
    EXPECT_EQ(stock.getPriceHistory()[1], 110.0);
    EXPECT_EQ(stock.getHighestPrice(), 110.0);
    EXPECT_EQ(stock.getLowestPrice(), 100.0);

    // Update to new highest price
    stock.updatePrice(120.0);
    EXPECT_EQ(stock.getHighestPrice(), 120.0);

    // Update to new lowest price
    stock.updatePrice(90.0);
    EXPECT_EQ(stock.getLowestPrice(), 90.0);

    // Check price history limit
    for (int i = 0; i < 1000; i++) {
        stock.updatePrice(100.0 + i);
    }
    EXPECT_LE(stock.getPriceHistory().size(), 1000);
}

TEST_F(StockTest, DailyChangeCalculation) {
    Stock stock(company, 100.0);

    // Open day at 100.0
    stock.openDay();
    EXPECT_EQ(stock.getOpenPrice(), 100.0);

    // Update price during the day
    stock.updatePrice(110.0);
    stock.calculateDailyChange();

    // Check daily change
    EXPECT_EQ(stock.getDayChangeAmount(), 10.0);
    EXPECT_EQ(stock.getDayChangePercent(), 10.0);

    // Close day and open new day
    stock.closeDay();
    stock.openDay();

    // Check that open price is updated
    EXPECT_EQ(stock.getOpenPrice(), 110.0);
    EXPECT_EQ(stock.getDayChangeAmount(), 0.0);
    EXPECT_EQ(stock.getDayChangePercent(), 0.0);

    // Update price and check new daily change
    stock.updatePrice(100.0);
    stock.calculateDailyChange();
    EXPECT_EQ(stock.getDayChangeAmount(), -10.0);
    EXPECT_NEAR(stock.getDayChangePercent(), -9.09, 0.01); // -10.0/110.0 * 100.0 ≈ -9.09%
}

TEST_F(StockTest, MarketAndSectorInfluence) {
    Stock stock(company, 100.0);

    // Set custom influence values
    stock.setMarketInfluence(0.7);
    stock.setSectorInfluence(0.2);

    // Test market influence
    double marketTrend = 0.1; // 10% market growth
    stock.updatePriceWithMarketInfluence(marketTrend);
    double expectedPrice = 100.0 * (1.0 + marketTrend * 0.7);
    EXPECT_NEAR(stock.getCurrentPrice(), expectedPrice, 0.001);

    // Reset stock price
    stock = Stock(company, 100.0);
    stock.setMarketInfluence(0.7);
    stock.setSectorInfluence(0.2);

    // Test sector influence
    double sectorTrend = 0.05; // 5% sector growth
    stock.updatePriceWithSectorInfluence(sectorTrend);
    expectedPrice = 100.0 * (1.0 + sectorTrend * 0.2);
    EXPECT_NEAR(stock.getCurrentPrice(), expectedPrice, 0.001);
}

TEST_F(StockTest, NewsImpact) {
    Stock stock(company, 100.0);

    // Test news impact
    double newsImpact = 0.05; // 5% positive news
    stock.updatePriceWithNewsImpact(newsImpact);
    double expectedPrice = 100.0 * (1.0 + newsImpact);
    EXPECT_NEAR(stock.getCurrentPrice(), expectedPrice, 0.001);

    // Test negative news impact
    newsImpact = -0.05; // 5% negative news
    stock = Stock(company, 100.0);
    stock.updatePriceWithNewsImpact(newsImpact);
    expectedPrice = 100.0 * (1.0 + newsImpact);
    EXPECT_NEAR(stock.getCurrentPrice(), expectedPrice, 0.001);
}

TEST_F(StockTest, VolatilityImpact) {
    Stock stock(company, 100.0);

    // Since volatility uses random numbers, we can only check that the price changes
    double initialPrice = stock.getCurrentPrice();
    stock.updatePriceWithVolatility(0.5);
    EXPECT_NE(stock.getCurrentPrice(), initialPrice);
}

TEST_F(StockTest, PriceMovementGeneration) {
    Stock stock(company, 100.0);
    stock.setMarketInfluence(0.6);
    stock.setSectorInfluence(0.3);

    double volatility = 0.05;
    double marketTrend = 0.02; // 2% market growth
    double sectorTrend = 0.01; // 1% sector growth

    // Generate price movement
    double newPrice = stock.generatePriceMovement(volatility, marketTrend, sectorTrend);

    // Check that new price is different from current price
    EXPECT_NE(newPrice, stock.getCurrentPrice());

    // Hard to test exact value due to random component, but should be in a reasonable range
    EXPECT_GT(newPrice, 0.0);
}

TEST_F(StockTest, DateMethods) {
    Stock stock(company, 100.0);

    // Test close day with specific date
    Date closeDate(15, 3, 2023);
    stock.closeDay(closeDate);
    EXPECT_EQ(stock.getLastUpdateDate(), closeDate);

    // Test open day with specific date
    Date openDate(16, 3, 2023);
    stock.openDay(openDate);
    EXPECT_EQ(stock.getLastUpdateDate(), openDate);

    // Test original methods still work
    stock.closeDay();
    EXPECT_EQ(stock.getLastUpdateDate(), Date()); // Default date

    stock.openDay();
    EXPECT_EQ(stock.getLastUpdateDate(), Date()); // Default date
}

TEST_F(StockTest, Serialization) {
    Stock stock(company, 100.0);
    stock.updatePrice(110.0);

    Date currentDate(5, 3, 2023);
    stock.closeDay(currentDate);

    // Serialize to JSON
    nlohmann::json j = stock.toJson();

    // Deserialize from JSON
    Stock deserializedStock = Stock::fromJson(j, company);

    // Check if deserialized stock matches original
    EXPECT_EQ(deserializedStock.getCurrentPrice(), stock.getCurrentPrice());
    EXPECT_EQ(deserializedStock.getPriceHistory().size(), stock.getPriceHistory().size());
    EXPECT_EQ(deserializedStock.getHighestPrice(), stock.getHighestPrice());
    EXPECT_EQ(deserializedStock.getLowestPrice(), stock.getLowestPrice());
    EXPECT_EQ(deserializedStock.getPreviousClosePrice(), stock.getPreviousClosePrice());
    EXPECT_EQ(deserializedStock.getOpenPrice(), stock.getOpenPrice());
    EXPECT_EQ(deserializedStock.getLastUpdateDate(), stock.getLastUpdateDate());
}