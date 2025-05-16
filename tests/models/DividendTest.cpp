#include <gtest/gtest.h>
#include "../../src/core/Game.hpp"
#include "../../src/core/Market.hpp"
#include "../../src/core/Player.hpp"
#include "../../src/models/Company.hpp"

using namespace StockMarketSimulator;

class DividendTest : public ::testing::Test {
protected:
    void SetUp() override {
        game = std::make_shared<Game>();
        game->initialize("TestPlayer", 10000.0);
        game->start();
    }

    std::shared_ptr<Game> game;
};

TEST_F(DividendTest, TestDividendPayment) {
    // Get references to market and player
    auto market = game->getMarket();
    auto player = game->getPlayer();
    
    // Find a company with dividends
    std::shared_ptr<Company> dividendCompany = nullptr;
    for (const auto& company : market->getCompanies()) {
        const auto& policy = company->getDividendPolicy();
        if (policy.annualDividendRate > 0 && policy.paymentFrequency > 0) {
            dividendCompany = company;
            break;
        }
    }
    
    ASSERT_NE(dividendCompany, nullptr) << "No company with dividends found";
    
    // Buy some shares of this company
    const int SHARES_TO_BUY = 100;
    bool buyResult = player->buyStock(dividendCompany, SHARES_TO_BUY);
    ASSERT_TRUE(buyResult) << "Failed to buy shares";
    
    // Record initial dividend amount
    double initialDividends = player->getPortfolio()->getTotalDividendsReceived();
    
    // Calculate how many days until next dividend payment
    const auto& policy = dividendCompany->getDividendPolicy();
    int daysUntilDividend = 0;
    
    // Simulate enough days to ensure a dividend payment
    // We'll simulate up to 400 days (more than a year) to ensure at least one payment
    bool dividendPaid = false;
    double expectedDividendAmount = 0;
    
    for (int day = 0; day < 400 && !dividendPaid; day++) {
        // Advance one day
        game->simulateDay();
        
        // Check if dividend was paid
        double currentDividends = player->getPortfolio()->getTotalDividendsReceived();
        if (currentDividends > initialDividends) {
            expectedDividendAmount = policy.calculateDividendAmount() * SHARES_TO_BUY;
            double actualDividendAmount = currentDividends - initialDividends;
            
            // Verify the dividend amount (with some tolerance for floating point)
            EXPECT_NEAR(actualDividendAmount, expectedDividendAmount, 0.001) 
                << "Dividend amount doesn't match expected value";
                
            dividendPaid = true;
        }
    }
    
    ASSERT_TRUE(dividendPaid) << "No dividend was paid after simulating 400 days";
}

// Test to verify dividend frequency
TEST_F(DividendTest, TestDividendFrequency) {
    auto market = game->getMarket();
    auto player = game->getPlayer();
    
    // Find a company with quarterly dividends (frequency = 4)
    std::shared_ptr<Company> dividendCompany = nullptr;
    for (const auto& company : market->getCompanies()) {
        const auto& policy = company->getDividendPolicy();
        if (policy.annualDividendRate > 0 && policy.paymentFrequency == 4) {
            dividendCompany = company;
            break;
        }
    }
    
    ASSERT_NE(dividendCompany, nullptr) << "No company with quarterly dividends found";
    
    // Buy some shares
    const int SHARES_TO_BUY = 100;
    bool buyResult = player->buyStock(dividendCompany, SHARES_TO_BUY);
    ASSERT_TRUE(buyResult) << "Failed to buy shares";
    
    // Track dividend payments
    std::vector<int> paymentDays;
    double lastDividendAmount = player->getPortfolio()->getTotalDividendsReceived();
    
    // Simulate 400 days (more than a year)
    for (int day = 0; day < 400; day++) {
        // Advance one day
        game->simulateDay();
        
        // Check if dividend was paid
        double currentDividends = player->getPortfolio()->getTotalDividendsReceived();
        if (currentDividends > lastDividendAmount) {
            paymentDays.push_back(day);
            lastDividendAmount = currentDividends;
        }
    }
    
    // We should have at least 4 payments (quarterly over a year)
    ASSERT_GE(paymentDays.size(), 4) << "Expected at least 4 dividend payments";
    
    // Check intervals between payments (should be around 91 days for quarterly)
    for (size_t i = 1; i < paymentDays.size(); i++) {
        int interval = paymentDays[i] - paymentDays[i-1];
        // Allow some leeway (90-92 days)
        EXPECT_NEAR(interval, 91, 1) << "Unexpected interval between dividend payments";
    }
}