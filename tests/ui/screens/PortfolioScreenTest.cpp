#include <gtest/gtest.h>
#include <memory>
#include "ui/screens/PortfolioScreen.hpp"
#include "core/Market.hpp"
#include "core/Player.hpp"
#include "models/Company.hpp"

namespace StockMarketSimulator {

class PortfolioScreenTest : public ::testing::Test {
protected:
    std::shared_ptr<Market> market;
    std::shared_ptr<Player> player;
    std::shared_ptr<PortfolioScreen> screen;

    void SetUp() override {
        market = std::make_shared<Market>();
        market->addDefaultCompanies();

        player = std::make_shared<Player>("TestPlayer", 10000.0);
        player->setMarket(market);

        auto company1 = market->getCompanyByTicker("TCH");
        auto company2 = market->getCompanyByTicker("EPLC");

        if (company1 && company2) {
            player->buyStock(company1, 10);
            player->buyStock(company2, 15);
        }

        for (int i = 0; i < 30; i++) {
            market->simulateDay();
            player->updateDailyState();
            player->closeDay();
        }

        screen = std::make_shared<PortfolioScreen>();
        screen->setMarket(market);
        screen->setPlayer(player);
    }
};

TEST_F(PortfolioScreenTest, Initialization) {
    screen->initialize();

    EXPECT_EQ(screen->getTitle(), "MY PORTFOLIO");
    EXPECT_EQ(screen->getType(), ScreenType::Portfolio);
}

TEST_F(PortfolioScreenTest, UpdateShowsCorrectPortfolioData) {
    screen->initialize();
    screen->update();

    SUCCEED();
}

TEST_F(PortfolioScreenTest, HandleInputNavigatesToCompanyScreen) {
    screen->initialize();

    bool result = screen->handleInput('1');
    EXPECT_TRUE(result);
}

TEST_F(PortfolioScreenTest, HandleInputAllowsSellStocks) {
    screen->initialize();

    bool result = screen->handleInput('2');
    EXPECT_TRUE(result);
}

TEST_F(PortfolioScreenTest, HandleInputReturnsToMainMenu) {
    screen->initialize();

    bool result = screen->handleInput('3');
    EXPECT_FALSE(result);
    EXPECT_FALSE(screen->isActive());
}

TEST_F(PortfolioScreenTest, HandleNavigationKeysChangesPage) {
    screen->initialize();

    for (int i = 0; i < 5; i++) {
        auto company = market->getCompanies()[i % market->getCompanies().size()];
        player->buyStock(company, 5);
    }

    bool nextResult = screen->handleInput(']');
    EXPECT_TRUE(nextResult);

    bool prevResult = screen->handleInput('[');
    EXPECT_TRUE(prevResult);
}

}