#include <gtest/gtest.h>
#include "../../src/core/Game.hpp"

using namespace StockMarketSimulator;

class GameTest : public ::testing::Test {
protected:
    std::shared_ptr<Game> game;
    
    void SetUp() override {
        game = std::make_shared<Game>();
    }
};

// Test initialization of game components
TEST_F(GameTest, Initialization) {
    ASSERT_NO_THROW(game->initialize("TestPlayer", 5000.0));
    
    EXPECT_EQ(game->getStatus(), GameStatus::NotStarted);
    EXPECT_EQ(game->getGameSpeed(), 1);
    EXPECT_EQ(game->getSimulatedDays(), 0);
    
    EXPECT_NE(game->getMarket(), nullptr);
    EXPECT_NE(game->getPlayer(), nullptr);
    EXPECT_NE(game->getNewsService(), nullptr);
    EXPECT_NE(game->getPriceService(), nullptr);
    EXPECT_NE(game->getSaveService(), nullptr);
    
    EXPECT_EQ(game->getPlayer()->getName(), "TestPlayer");
    EXPECT_NEAR(game->getPlayer()->getPortfolio()->getCashBalance(), 5000.0, 0.001);
}

// Test game state transitions
TEST_F(GameTest, GameStateManagement) {
    game->initialize();
    
    EXPECT_TRUE(game->start());
    EXPECT_EQ(game->getStatus(), GameStatus::Running);
    
    game->pause();
    EXPECT_EQ(game->getStatus(), GameStatus::Paused);
    
    game->resume();
    EXPECT_EQ(game->getStatus(), GameStatus::Running);
    
    game->end();
    EXPECT_EQ(game->getStatus(), GameStatus::Ended);
}

// Test day simulation
TEST_F(GameTest, DaySimulation) {
    game->initialize();
    game->start();
    
    EXPECT_TRUE(game->simulateDay());
    EXPECT_EQ(game->getSimulatedDays(), 1);
    
    int initialDays = game->getSimulatedDays();
    EXPECT_TRUE(game->simulateDays(5));
    EXPECT_EQ(game->getSimulatedDays(), initialDays + 5);
}

// Test trading operations
TEST_F(GameTest, TradingOperations) {
    game->initialize();
    game->start();
    
    auto market = game->getMarket();
    auto player = game->getPlayer();
    
    // Ensure we have some companies
    ASSERT_FALSE(market->getCompanies().empty());
    
    // Get the first company
    auto company = market->getCompanies().front();
    std::string ticker = company->getTicker();
    
    double initialBalance = player->getPortfolio()->getCashBalance();
    
    // Buy some stocks
    EXPECT_TRUE(game->buyStock(ticker, 10));
    EXPECT_LT(player->getPortfolio()->getCashBalance(), initialBalance);
    EXPECT_EQ(player->getPortfolio()->getPositionQuantity(ticker), 10);
    
    // Sell some stocks
    EXPECT_TRUE(game->sellStock(ticker, 5));
    EXPECT_EQ(player->getPortfolio()->getPositionQuantity(ticker), 5);
}

// Test loan operations
TEST_F(GameTest, LoanOperations) {
    game->initialize();
    game->start();
    
    auto player = game->getPlayer();
    double initialBalance = player->getPortfolio()->getCashBalance();
    
    // Take a loan
    EXPECT_TRUE(game->takeLoan(1000.0, 0.05, 30, "Test Loan"));
    EXPECT_NEAR(player->getPortfolio()->getCashBalance(), initialBalance + 1000.0, 0.001);
    
    // Verify the loan exists
    EXPECT_FALSE(player->getLoans().empty());
    
    // Repay the loan
    EXPECT_TRUE(game->repayLoan(0, 1000.0));
    EXPECT_NEAR(player->getPortfolio()->getCashBalance(), initialBalance, 0.001);
}

// Test invalid operations
TEST_F(GameTest, InvalidOperations) {
    // Try operations on uninitialized game
    EXPECT_FALSE(game->buyStock("ABC", 10));
    EXPECT_FALSE(game->sellStock("ABC", 10));
    EXPECT_FALSE(game->takeLoan(1000.0, 0.05, 30));
    
    game->initialize();
    game->start();
    
    // Invalid ticker
    EXPECT_FALSE(game->buyStock("INVALID", 10));
    EXPECT_FALSE(game->sellStock("INVALID", 10));
    
    // Invalid loan index
    EXPECT_FALSE(game->repayLoan(99, 1000.0));
}

// Test game speed setting
TEST_F(GameTest, GameSpeedSetting) {
    game->initialize();
    
    EXPECT_EQ(game->getGameSpeed(), 1);
    
    game->setGameSpeed(5);
    EXPECT_EQ(game->getGameSpeed(), 5);
    
    // Invalid speed should not change the value
    game->setGameSpeed(0);
    EXPECT_EQ(game->getGameSpeed(), 5);
    
    game->setGameSpeed(-1);
    EXPECT_EQ(game->getGameSpeed(), 5);
}

// Test error handling
TEST_F(GameTest, ErrorHandling) {
    // Initially no error
    EXPECT_TRUE(game->getLastError().empty());
    
    // Trying to simulate day when game is not running should set an error
    EXPECT_FALSE(game->simulateDay());
    EXPECT_FALSE(game->getLastError().empty());
}
