#include <gtest/gtest.h>
#include <memory>
#include "../../src/models/Portfolio.hpp"
#include "../../src/models/Company.hpp"
#include "../../src/models/Stock.hpp"
#include "../../src/utils/Date.hpp"

using namespace StockMarketSimulator;

class PortfolioDateTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a company for testing
        company = std::make_shared<Company>(
            "TestCorp", "TEST",
            "A test company",
            Sector::Technology,
            100.0, 0.5,
            DividendPolicy(2.0, 4)
        );

        // Create a portfolio with initial balance
        portfolio = std::make_unique<Portfolio>(10000.0);
    }

    std::shared_ptr<Company> company;
    std::unique_ptr<Portfolio> portfolio;
};

// Test buying stocks on different dates
TEST_F(PortfolioDateTest, BuyStocksOnDifferentDates) {
    // Buy stocks on different dates
    Date day1(1, 3, 2023);
    Date day2(2, 3, 2023);
    Date day3(3, 3, 2023);

    // Buy 10 stocks on day 1
    EXPECT_TRUE(portfolio->buyStock(company, 10, 100.0, 0.01, day1));
    portfolio->closeDay(day1);

    // Buy 5 more stocks on day 2
    EXPECT_TRUE(portfolio->buyStock(company, 5, 110.0, 0.01, day2));
    portfolio->closeDay(day2);

    // Buy 3 more stocks on day 3
    EXPECT_TRUE(portfolio->buyStock(company, 3, 120.0, 0.01, day3));
    portfolio->closeDay(day3);

    // Check portfolio history
    const auto& history = portfolio->getHistory();
    ASSERT_EQ(3, history.size());

    // Check that dates are correct
    EXPECT_EQ(day1.getDay(), history[0].date.getDay());
    EXPECT_EQ(day1.getMonth(), history[0].date.getMonth());
    EXPECT_EQ(day1.getYear(), history[0].date.getYear());

    EXPECT_EQ(day2.getDay(), history[1].date.getDay());
    EXPECT_EQ(day2.getMonth(), history[1].date.getMonth());
    EXPECT_EQ(day2.getYear(), history[1].date.getYear());

    EXPECT_EQ(day3.getDay(), history[2].date.getDay());
    EXPECT_EQ(day3.getMonth(), history[2].date.getMonth());
    EXPECT_EQ(day3.getYear(), history[2].date.getYear());
}

// Test selling stocks and checking history
TEST_F(PortfolioDateTest, SellStocksAndCheckHistory) {
    // Setup initial portfolio
    Date day1(1, 3, 2023);
    EXPECT_TRUE(portfolio->buyStock(company, 20, 100.0, 0.01, day1));
    portfolio->closeDay(day1);

    // Sell some stocks on a different date
    Date day2(5, 3, 2023);
    EXPECT_TRUE(portfolio->sellStock(company, 10, 120.0, 0.01, day2));
    portfolio->closeDay(day2);

    // Check portfolio history
    const auto& history = portfolio->getHistory();
    ASSERT_EQ(2, history.size());

    // Check that dates are correct
    EXPECT_EQ(day1.getDay(), history[0].date.getDay());
    EXPECT_EQ(day1.getMonth(), history[0].date.getMonth());
    EXPECT_EQ(day1.getYear(), history[0].date.getYear());

    EXPECT_EQ(day2.getDay(), history[1].date.getDay());
    EXPECT_EQ(day2.getMonth(), history[1].date.getMonth());
    EXPECT_EQ(day2.getYear(), history[1].date.getYear());
}

// Test JSON serialization and deserialization with dates
TEST_F(PortfolioDateTest, SaveAndLoadWithDates) {
    // Setup initial portfolio
    Date day1(1, 3, 2023);
    EXPECT_TRUE(portfolio->buyStock(company, 15, 100.0, 0.01, day1));
    portfolio->closeDay(day1);

    // Save to JSON
    nlohmann::json j = portfolio->toJson();

    // Create a new portfolio by loading from JSON
    std::vector<std::shared_ptr<Company>> companies = {company};
    Portfolio loadedPortfolio = Portfolio::fromJson(j, companies);

    // Check that the history has the correct date
    const auto& history = loadedPortfolio.getHistory();
    ASSERT_EQ(1, history.size());
    EXPECT_EQ(day1.getDay(), history[0].date.getDay());
    EXPECT_EQ(day1.getMonth(), history[0].date.getMonth());
    EXPECT_EQ(day1.getYear(), history[0].date.getYear());
}

// Test backward compatibility - loading from old format
TEST_F(PortfolioDateTest, BackwardCompatibility) {
    // Create history entry in old format (with day as int)
    nlohmann::json historyJson;
    historyJson["day"] = 25;  // Old format using day number
    historyJson["total_value"] = 12000.0;
    historyJson["cash_balance"] = 5000.0;
    historyJson["total_return"] = 2000.0;
    historyJson["total_return_percent"] = 20.0;

    // Load history entry
    PortfolioHistory historyEntry = PortfolioHistory::fromJson(historyJson);

    // Check that it converted to the correct date
    Date expectedDate = Date::fromDayNumber(25);
    EXPECT_EQ(expectedDate.getDay(), historyEntry.date.getDay());
    EXPECT_EQ(expectedDate.getMonth(), historyEntry.date.getMonth());
    EXPECT_EQ(expectedDate.getYear(), historyEntry.date.getYear());
}

// Test period returns across different dates
TEST_F(PortfolioDateTest, PeriodReturnAcrossDifferentDates) {
    // Create a series of history entries
    Date day1(1, 3, 2023);
    Date day2(15, 3, 2023);
    Date day3(31, 3, 2023);
    Date day4(15, 4, 2023);

    // Simulate buying and price increases over time
    EXPECT_TRUE(portfolio->buyStock(company, 10, 100.0, 0.01, day1));
    company->getStock()->updatePrice(110.0); // Price up 10%
    portfolio->updatePositionValues();
    portfolio->closeDay(day1);

    company->getStock()->updatePrice(120.0); // Price up another 9.1%
    portfolio->updatePositionValues();
    portfolio->closeDay(day2);

    company->getStock()->updatePrice(130.0); // Price up another 8.3%
    portfolio->updatePositionValues();
    portfolio->closeDay(day3);

    company->getStock()->updatePrice(140.0); // Price up another 7.7%
    portfolio->updatePositionValues();
    portfolio->closeDay(day4);

    // Check period returns (still using the days count)
    // The expected values are the difference between current value and historical value
    // So for getPeriodReturn(3): difference between day4 and day1 = 10 stocks * (140-110) = 300
    // For getPeriodReturn(2): difference between day4 and day2 = 10 stocks * (140-120) = 200
    // For getPeriodReturn(1): difference between day4 and day3 = 10 stocks * (140-130) = 100
    EXPECT_NEAR(300.0, portfolio->getPeriodReturn(3), 0.1);
    EXPECT_NEAR(200.0, portfolio->getPeriodReturn(2), 0.1);
    EXPECT_NEAR(100.0, portfolio->getPeriodReturn(1), 0.1);
}

// Test multiple transactions on the same date
TEST_F(PortfolioDateTest, MultipleTransactionsOnSameDate) {
    Date transactionDate(5, 4, 2023);

    // Buy 10 stocks
    EXPECT_TRUE(portfolio->buyStock(company, 10, 100.0, 0.01, transactionDate));

    // Buy 5 more stocks
    EXPECT_TRUE(portfolio->buyStock(company, 5, 102.0, 0.01, transactionDate));

    // Sell 3 stocks
    EXPECT_TRUE(portfolio->sellStock(company, 3, 103.0, 0.01, transactionDate));

    // Close the day and record history
    portfolio->closeDay(transactionDate);

    // Check position
    EXPECT_EQ(12, portfolio->getPositionQuantity(company->getTicker()));

    // Check history
    const auto& history = portfolio->getHistory();
    ASSERT_EQ(1, history.size());
    EXPECT_EQ(transactionDate.getDay(), history[0].date.getDay());
    EXPECT_EQ(transactionDate.getMonth(), history[0].date.getMonth());
    EXPECT_EQ(transactionDate.getYear(), history[0].date.getYear());
}

// Test dates across month boundaries
TEST_F(PortfolioDateTest, DatesAcrossMonthBoundaries) {
    Date lastDayOfMarch(31, 3, 2023);
    Date firstDayOfApril(1, 4, 2023);

    // Buy stocks on the last day of March
    EXPECT_TRUE(portfolio->buyStock(company, 10, 100.0, 0.01, lastDayOfMarch));
    portfolio->closeDay(lastDayOfMarch);

    // Buy more stocks on the first day of April
    EXPECT_TRUE(portfolio->buyStock(company, 5, 105.0, 0.01, firstDayOfApril));
    portfolio->closeDay(firstDayOfApril);

    // Check history
    const auto& history = portfolio->getHistory();
    ASSERT_EQ(2, history.size());

    // Check that dates are correct
    EXPECT_EQ(31, history[0].date.getDay());
    EXPECT_EQ(3, history[0].date.getMonth());
    EXPECT_EQ(2023, history[0].date.getYear());

    EXPECT_EQ(1, history[1].date.getDay());
    EXPECT_EQ(4, history[1].date.getMonth());
    EXPECT_EQ(2023, history[1].date.getYear());
}