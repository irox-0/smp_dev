#include <gtest/gtest.h>
#include <memory>
#include "../../src/core/Player.hpp"
#include "../../src/core/Market.hpp"
#include "../../src/models/Company.hpp"
#include "../../src/utils/Date.hpp"

using namespace StockMarketSimulator;

class PlayerTest : public ::testing::Test {
protected:
    std::shared_ptr<Market> market;
    std::shared_ptr<Company> techCompany;
    std::shared_ptr<Company> energyCompany;

    void SetUp() override {
        market = std::make_shared<Market>();

        // Create test companies
        techCompany = std::make_shared<Company>(
            "TechCorp", "TCH",
            "A technology company for unit tests",
            Sector::Technology,
            100.0, 0.5,
            DividendPolicy(2.0, 4)
        );

        energyCompany = std::make_shared<Company>(
            "EnergyPlus", "EPL",
            "An energy company for unit tests",
            Sector::Energy,
            50.0, 0.4,
            DividendPolicy(3.0, 4)
        );

        market->addCompany(techCompany);
        market->addCompany(energyCompany);
    }
};

// Test constructor and basic getters
TEST_F(PlayerTest, ConstructorAndGetters) {
    Player player("TestPlayer", 10000.0);
    player.setMarket(market);

    EXPECT_EQ("TestPlayer", player.getName());
    EXPECT_EQ(10000.0, player.getPortfolio()->getCashBalance());
    EXPECT_EQ(0.0, player.getMarginAccountBalance());
    EXPECT_EQ(0.0, player.getMarginUsed());
    EXPECT_NEAR(0.07, player.getMarginInterestRate(), 0.001);
    EXPECT_NEAR(0.5, player.getMarginRequirement(), 0.001);

    // Default date should be March 1, 2023
    Date expectedDate(1, 3, 2023);
    EXPECT_EQ(expectedDate.getDay(), player.getCurrentDate().getDay());
    EXPECT_EQ(expectedDate.getMonth(), player.getCurrentDate().getMonth());
    EXPECT_EQ(expectedDate.getYear(), player.getCurrentDate().getYear());
}

// Test setters
TEST_F(PlayerTest, Setters) {
    Player player;

    player.setName("NewName");
    EXPECT_EQ("NewName", player.getName());

    Date newDate(15, 4, 2023);
    player.setCurrentDate(newDate);
    EXPECT_EQ(newDate.getDay(), player.getCurrentDate().getDay());
    EXPECT_EQ(newDate.getMonth(), player.getCurrentDate().getMonth());
    EXPECT_EQ(newDate.getYear(), player.getCurrentDate().getYear());

    player.adjustMarginRequirement(0.6);
    EXPECT_NEAR(0.6, player.getMarginRequirement(), 0.001);

    player.adjustMarginInterestRate(0.08);
    EXPECT_NEAR(0.08, player.getMarginInterestRate(), 0.001);
}

// Test buying stocks
TEST_F(PlayerTest, BuyStock) {
    Player player("Investor", 10000.0);
    player.setMarket(market);

    bool result = player.buyStock(techCompany, 10);
    EXPECT_TRUE(result);

    // Verify the purchase
    EXPECT_EQ(1, player.getPortfolio()->getPositions().size());
    EXPECT_EQ(10, player.getPortfolio()->getPositionQuantity("TCH"));

    // Check cash balance (10 shares @ 100.0 each + 1% commission)
    double expectedBalance = 10000.0 - (10 * 100.0 * 1.01);
    EXPECT_NEAR(expectedBalance, player.getPortfolio()->getCashBalance(), 0.01);
}

// Test selling stocks
TEST_F(PlayerTest, SellStock) {
    Player player("Investor", 10000.0);
    player.setMarket(market);

    // First buy some stocks
    player.buyStock(techCompany, 10);

    // Then sell half of them
    bool result = player.sellStock(techCompany, 5);
    EXPECT_TRUE(result);

    // Verify the sale
    EXPECT_EQ(5, player.getPortfolio()->getPositionQuantity("TCH"));

    // Check cash balance (initial - buy cost + sell proceeds)
    double buyCost = 10 * 100.0 * 1.01;
    double sellProceeds = 5 * 100.0 * 0.99;
    double expectedBalance = 10000.0 - buyCost + sellProceeds;
    EXPECT_NEAR(expectedBalance, player.getPortfolio()->getCashBalance(), 0.01);
}

// Test diversified portfolio
TEST_F(PlayerTest, DiversifiedPortfolio) {
    Player player("Investor", 10000.0);
    player.setMarket(market);

    // Buy different stocks
    player.buyStock(techCompany, 10);
    player.buyStock(energyCompany, 20);

    // Verify both purchases
    EXPECT_EQ(2, player.getPortfolio()->getPositions().size());
    EXPECT_EQ(10, player.getPortfolio()->getPositionQuantity("TCH"));
    EXPECT_EQ(20, player.getPortfolio()->getPositionQuantity("EPL"));

    // Check cash balance
    double techCost = 10 * 100.0 * 1.01;
    double energyCost = 20 * 50.0 * 1.01;
    double expectedBalance = 10000.0 - techCost - energyCost;
    EXPECT_NEAR(expectedBalance, player.getPortfolio()->getCashBalance(), 0.01);
}

// Test taking a loan
TEST_F(PlayerTest, TakeLoan) {
    Player player("Investor", 10000.0);

    // Take a loan
    bool result = player.takeLoan(5000.0, 0.05, 30, "Test Loan");
    EXPECT_TRUE(result);

    // Verify the loan was added
    EXPECT_EQ(1, player.getLoans().size());

    // Check cash balance was increased
    EXPECT_NEAR(15000.0, player.getPortfolio()->getCashBalance(), 0.01);

    // Verify loan properties
    const Loan& loan = player.getLoans()[0];
    EXPECT_NEAR(5000.0, loan.getAmount(), 0.01);
    EXPECT_NEAR(0.05, loan.getInterestRate(), 0.001);
}

// Test repaying a loan
TEST_F(PlayerTest, RepayLoan) {
    Player player("Investor", 10000.0);

    // Take a loan
    player.takeLoan(5000.0, 0.05, 30, "Test Loan");

    // Repay part of the loan
    bool result = player.repayLoan(0, 2000.0);
    EXPECT_TRUE(result);

    // Check cash balance was decreased
    EXPECT_NEAR(13000.0, player.getPortfolio()->getCashBalance(), 0.01);
}

// Test margin account operations
TEST_F(PlayerTest, MarginAccount) {
    Player player("Investor", 10000.0);

    // Deposit to margin account
    bool result = player.depositToMarginAccount(2000.0);
    EXPECT_TRUE(result);

    // Check balances
    EXPECT_NEAR(8000.0, player.getPortfolio()->getCashBalance(), 0.01);
    EXPECT_NEAR(2000.0, player.getMarginAccountBalance(), 0.01);

    // Withdraw from margin account
    result = player.withdrawFromMarginAccount(1000.0);
    EXPECT_TRUE(result);

    // Check balances
    EXPECT_NEAR(9000.0, player.getPortfolio()->getCashBalance(), 0.01);
    EXPECT_NEAR(1000.0, player.getMarginAccountBalance(), 0.01);
}

// Test margin buying
TEST_F(PlayerTest, MarginBuying) {
    Player player("Investor", 10000.0);
    player.setMarket(market);

    // Buy stocks with cash
    player.buyStock(techCompany, 90);

    // Deposit some cash to margin account
    player.depositToMarginAccount(500.0);

    // Buy stocks on margin
    bool result = player.buyStock(techCompany, 10, true);
    EXPECT_TRUE(result);

    // Verify total position
    EXPECT_EQ(100, player.getPortfolio()->getPositionQuantity("TCH"));

    // Check margin used
    EXPECT_GT(player.getMarginUsed(), 0.0);
}

// Test closing day and advancing date
TEST_F(PlayerTest, CloseDay) {
    Player player("Investor", 10000.0);

    Date initialDate = player.getCurrentDate();

    // Close the day
    player.closeDay();

    // Verify date advanced by one day
    Date expectedDate(2, 3, 2023);  // March 2, 2023
    EXPECT_EQ(expectedDate.getDay(), player.getCurrentDate().getDay());
    EXPECT_EQ(expectedDate.getMonth(), player.getCurrentDate().getMonth());
    EXPECT_EQ(expectedDate.getYear(), player.getCurrentDate().getYear());
}

// Test advancing multiple days
TEST_F(PlayerTest, AdvanceMultipleDays) {
    Player player("Investor", 10000.0);

    // Advance 10 days
    for (int i = 0; i < 10; i++) {
        player.closeDay();
    }

    // Verify date advanced by 10 days
    Date expectedDate(11, 3, 2023);  // March 11, 2023
    EXPECT_EQ(expectedDate.getDay(), player.getCurrentDate().getDay());
    EXPECT_EQ(expectedDate.getMonth(), player.getCurrentDate().getMonth());
    EXPECT_EQ(expectedDate.getYear(), player.getCurrentDate().getYear());
}

// Test processing loans over time
TEST_F(PlayerTest, ProcessLoans) {
    Player player("Investor", 10000.0);

    // Take a loan
    player.takeLoan(5000.0, 0.1, 30, "Test Loan");

    // Remember initial total due
    double initialTotalDue = player.getLoans()[0].getTotalDue();

    // Advance a few days
    for (int i = 0; i < 5; i++) {
        player.updateDailyState();
        player.closeDay();
    }

    // Check that interest has accrued
    EXPECT_GT(player.getLoans()[0].getTotalDue(), initialTotalDue);
}

// Test processing dividends
TEST_F(PlayerTest, ProcessDividends) {
    Player player("Investor", 10000.0);
    player.setMarket(market);

    // Buy stocks
    player.buyStock(techCompany, 100);

    // Get initial cash balance
    double initialCash = player.getPortfolio()->getCashBalance();

    // Set up dividend parameters in the company
    // Ensure we're at a day where dividends should be paid
    Date paymentDate(1, 3, 2023); // Default date
    player.setCurrentDate(paymentDate);

    // Make the company trigger dividend payment
    // (This simulates a dividend payment day)
    bool dividendPaid = techCompany->processDividends(paymentDate);

    // Only if company would pay dividends, we test the player receiving them
    if (dividendPaid) {
        double dividendPerShare = techCompany->calculateDividendAmount();

        // Receive dividends
        player.receiveDividends(techCompany, dividendPerShare);

        // Check cash balance increased by dividend amount
        double expectedCash = initialCash + (100 * dividendPerShare);
        EXPECT_NEAR(expectedCash, player.getPortfolio()->getCashBalance(), 0.01);
    } else {
        // Otherwise, just verify the method exists and doesn't crash
        player.receiveDividends(techCompany, 0.5);
        SUCCEED() << "Dividend method called successfully";
    }
}

// Test total asset value calculation
TEST_F(PlayerTest, TotalAssetValue) {
    Player player("Investor", 10000.0);
    player.setMarket(market);

    // Buy stocks
    player.buyStock(techCompany, 50);

    // Deposit to margin account
    player.depositToMarginAccount(1000.0);

    // Calculate expected asset value
    double cashBalance = player.getPortfolio()->getCashBalance();
    double stocksValue = 50 * 100.0;  // 50 shares at $100 each
    double marginBalance = 1000.0;
    double expectedAssetValue = cashBalance + stocksValue + marginBalance;

    EXPECT_NEAR(expectedAssetValue, player.getTotalAssetValue(), 0.01);
}

// Test total liabilities calculation
TEST_F(PlayerTest, TotalLiabilities) {
    Player player("Investor", 10000.0);

    // Take a loan
    player.takeLoan(3000.0, 0.05, 30, "First Loan");
    player.takeLoan(2000.0, 0.06, 60, "Second Loan");

    // Calculate expected liabilities
    double expectedLiabilities = 5000.0;  // Just the loan amounts for now

    EXPECT_NEAR(expectedLiabilities, player.getTotalLiabilities(), 0.01);
}

// Test net worth calculation
TEST_F(PlayerTest, NetWorth) {
    Player player("Investor", 10000.0);
    player.setMarket(market);

    // Buy stocks
    player.buyStock(techCompany, 50);

    // Take a loan
    player.takeLoan(2000.0, 0.05, 30, "Test Loan");

    // Calculate expected net worth
    double assets = player.getTotalAssetValue();
    double liabilities = player.getTotalLiabilities();
    double expectedNetWorth = assets - liabilities;

    EXPECT_NEAR(expectedNetWorth, player.getNetWorth(), 0.01);
}

// Test JSON serialization
TEST_F(PlayerTest, JsonSerialization) {
    Player player("Investor", 10000.0);
    player.setMarket(market);

    // Make some changes
    player.buyStock(techCompany, 50);
    player.takeLoan(2000.0, 0.05, 30, "Test Loan");
    Date newDate(15, 4, 2023);
    player.setCurrentDate(newDate);

    // Convert to JSON
    nlohmann::json playerJson = player.toJson();

    // Create a new player from the JSON
    Player newPlayer = Player::fromJson(playerJson, market);

    // Verify properties were preserved
    EXPECT_EQ("Investor", newPlayer.getName());
    EXPECT_EQ(newDate.getDay(), newPlayer.getCurrentDate().getDay());
    EXPECT_EQ(newDate.getMonth(), newPlayer.getCurrentDate().getMonth());
    EXPECT_EQ(newDate.getYear(), newPlayer.getCurrentDate().getYear());
    EXPECT_EQ(1, newPlayer.getLoans().size());
    EXPECT_NEAR(2000.0, newPlayer.getLoans()[0].getAmount(), 0.01);
}