#include "gtest/gtest.h"
#include "ui/screens/MainScreen.hpp"
#include "core/Market.hpp"
#include "core/Player.hpp"
#include "services/NewsService.hpp"
#include <memory>

using namespace StockMarketSimulator;

class MainScreenTest : public ::testing::Test {
protected:
    std::shared_ptr<Market> market;
    std::shared_ptr<Player> player;
    std::shared_ptr<NewsService> newsService;
    std::shared_ptr<MainScreen> mainScreen;

    void SetUp() override {
        market = std::make_shared<Market>();
        market->addDefaultCompanies();

        player = std::make_shared<Player>("Test Player", 10000.0);
        player->setMarket(market);

        newsService = std::make_shared<NewsService>(market);
        newsService->initialize();

        // Generate some test data
        for (int i = 0; i < 10; i++) {
            market->simulateDay();
            player->updateDailyState();
            player->closeDay();
        }

        mainScreen = std::make_shared<MainScreen>();
        mainScreen->setMarket(market);
        mainScreen->setPlayer(player);
        mainScreen->setNewsService(newsService);
    }
};

TEST_F(MainScreenTest, Initialization) {
    mainScreen->initialize();
    EXPECT_EQ(mainScreen->getTitle(), "STOCK PLAYER - MAIN MENU");
    EXPECT_EQ(mainScreen->getNewsService().lock(), newsService);
}

TEST_F(MainScreenTest, UpdateTopStocks) {
    mainScreen->initialize();

    // Get companies and simulate extreme price changes
    auto companies = market->getCompanies();
    ASSERT_FALSE(companies.empty());

    // Force a big price change on first company
    double initialPrice = companies[0]->getStock()->getCurrentPrice();
    companies[0]->getStock()->updatePrice(initialPrice * 1.2); // 20% increase

    // Update screen
    mainScreen->update();

    // This is not a direct test (since updateTopStocks is private)
    // but we can test that handleInput works which indirectly tests the screen was updated
    EXPECT_TRUE(mainScreen->handleInput('1'));
}

TEST_F(MainScreenTest, UpdateLatestNews) {
    mainScreen->initialize();

    // Create some news
    auto news = newsService->generateDailyNews(3);

    // Update screen
    mainScreen->update();

    // Again, this is an indirect test
    EXPECT_TRUE(mainScreen->handleInput('3'));
}

TEST_F(MainScreenTest, HandleInputNavigation) {
    mainScreen->initialize();

    // Test all valid inputs
    EXPECT_TRUE(mainScreen->handleInput('1'));  // Market
    EXPECT_TRUE(mainScreen->handleInput('2'));  // Portfolio
    EXPECT_TRUE(mainScreen->handleInput('3'));  // News
    EXPECT_TRUE(mainScreen->handleInput('4'));  // Financial
    EXPECT_TRUE(mainScreen->handleInput('5'));  // Save/Load

    // Test invalid input
    EXPECT_TRUE(mainScreen->handleInput('9'));

    // Test exit - should return false
    EXPECT_FALSE(mainScreen->handleInput('0'));

    // Test escape key - should also exit
    EXPECT_FALSE(mainScreen->handleInput(27));
}
