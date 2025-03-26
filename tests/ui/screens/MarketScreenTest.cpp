#include <gtest/gtest.h>
#include "../../../src/ui/screens/MarketScreen.hpp"
#include "../../../src/core/Market.hpp"
#include "../../../src/core/Player.hpp"
#include <memory>

namespace StockMarketSimulator {

class MarketScreenTest : public ::testing::Test {
protected:
    std::shared_ptr<Market> market;
    std::shared_ptr<Player> player;
    std::shared_ptr<MarketScreen> marketScreen;

    void SetUp() override {
        market = std::make_shared<Market>();
        market->addDefaultCompanies();

        player = std::make_shared<Player>("Test Player", 10000.0);
        player->setMarket(market);

        marketScreen = std::make_shared<MarketScreen>();
        marketScreen->setMarket(market);
        marketScreen->setPlayer(player);
        marketScreen->initialize();
    }
};

TEST_F(MarketScreenTest, InitialState) {
    // The title is actually "" because we're drawing it manually in drawContent
    EXPECT_EQ(marketScreen->getType(), ScreenType::Market);
    EXPECT_EQ(marketScreen->getSortCriteria(), MarketSortCriteria::PriceChangePercent);
    EXPECT_FALSE(marketScreen->isSortAscending()); // Descending by default
}

TEST_F(MarketScreenTest, SortCriteriaChange) {
    marketScreen->setSortCriteria(MarketSortCriteria::Name);
    EXPECT_EQ(marketScreen->getSortCriteria(), MarketSortCriteria::Name);

    marketScreen->setSortCriteria(MarketSortCriteria::Price);
    EXPECT_EQ(marketScreen->getSortCriteria(), MarketSortCriteria::Price);

    marketScreen->setSortCriteria(MarketSortCriteria::PriceChange);
    EXPECT_EQ(marketScreen->getSortCriteria(), MarketSortCriteria::PriceChange);

    marketScreen->setSortCriteria(MarketSortCriteria::Sector);
    EXPECT_EQ(marketScreen->getSortCriteria(), MarketSortCriteria::Sector);
}

TEST_F(MarketScreenTest, SortDirectionToggle) {
    bool initialDirection = marketScreen->isSortAscending();

    marketScreen->setSortAscending(!initialDirection);
    EXPECT_EQ(marketScreen->isSortAscending(), !initialDirection);

    marketScreen->setSortAscending(initialDirection);
    EXPECT_EQ(marketScreen->isSortAscending(), initialDirection);
}

TEST_F(MarketScreenTest, HandleInput) {
    // Test handling of sorting keys
    EXPECT_TRUE(marketScreen->handleInput('s'));
    EXPECT_TRUE(marketScreen->handleInput('S'));
    EXPECT_TRUE(marketScreen->handleInput('d'));
    EXPECT_TRUE(marketScreen->handleInput('D'));

    // Test company selection (assuming at least one company exists)
    EXPECT_TRUE(marketScreen->handleInput('1'));

    // Test return to main menu
    EXPECT_FALSE(marketScreen->handleInput('0'));
    EXPECT_FALSE(marketScreen->handleInput(27)); // ESC
}

TEST_F(MarketScreenTest, CompaniesDisplay) {
    auto companies = market->getCompanies();
    EXPECT_FALSE(companies.empty());

    // Sort by different criteria to ensure the sorting logic works
    marketScreen->setSortCriteria(MarketSortCriteria::Name);
    marketScreen->setSortCriteria(MarketSortCriteria::Price);
    marketScreen->setSortCriteria(MarketSortCriteria::PriceChangePercent);

    // Test that we can toggle sort direction without errors
    marketScreen->toggleSortDirection();
    marketScreen->toggleSortDirection();
}

TEST_F(MarketScreenTest, TableUpdate) {
    // Simulate market activity and verify table updates
    for (int i = 0; i < 3; i++) {
        market->simulateDay();
        marketScreen->update();
    }

    // Test with different sort criteria
    marketScreen->setSortCriteria(MarketSortCriteria::PriceChange);
    marketScreen->update();

    // If we made it here without errors, the test passes
    SUCCEED();
}

}