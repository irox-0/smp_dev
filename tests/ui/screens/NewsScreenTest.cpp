#include <gtest/gtest.h>
#include "ui/screens/NewsScreen.hpp"
#include "services/NewsService.hpp"
#include "core/Market.hpp"
#include "core/Player.hpp"
#include <memory>

namespace StockMarketSimulator {

class NewsScreenTest : public ::testing::Test {
protected:
    std::shared_ptr<Market> market;
    std::shared_ptr<Player> player;
    std::shared_ptr<NewsService> newsService;
    std::shared_ptr<NewsScreen> newsScreen;

    void SetUp() override {
        market = std::make_shared<Market>();
        market->addDefaultCompanies();

        player = std::make_shared<Player>("Test Player", 10000.0);
        player->setMarket(market);

        newsService = std::make_shared<NewsService>(market);

        auto techCorp = market->getCompanyByTicker("TCH");
        auto energyCompany = market->getCompanyByTicker("EPLC");

        News globalNews(NewsType::Global, "Global Economic Update",
                       "Central Bank reduced rate to 3.5%", 0.02, 15);
        newsService->addCustomNews(globalNews);

        News companyNews(NewsType::Corporate, "Product Announcement",
                        "TechCorp announced a new product", 0.03, 14, techCorp);
        newsService->addCustomNews(companyNews);

        News sectorNews(NewsType::Sector, "Energy Sector Update",
                       "Oil prices rose by 2.5%", -0.01, 13, Sector::Energy);
        newsService->addCustomNews(sectorNews);

        newsScreen = std::make_shared<NewsScreen>();
        newsScreen->setMarket(market);
        newsScreen->setPlayer(player);
        newsScreen->setNewsService(newsService);
        newsScreen->initialize();
    }
};

TEST_F(NewsScreenTest, InitialState) {
    EXPECT_EQ(newsScreen->getTitle(), "NEWS");
    EXPECT_EQ(newsScreen->getType(), ScreenType::News);
    EXPECT_EQ(newsScreen->getCurrentFilter(), NewsFilter::All);
    EXPECT_EQ(newsScreen->getCurrentPage(), 0);
    EXPECT_GT(newsScreen->getNewsPerPage(), 0);
}

TEST_F(NewsScreenTest, FilterChange) {
    newsScreen->setCurrentFilter(NewsFilter::Global);
    EXPECT_EQ(newsScreen->getCurrentFilter(), NewsFilter::Global);

    newsScreen->setCurrentFilter(NewsFilter::Sector);
    EXPECT_EQ(newsScreen->getCurrentFilter(), NewsFilter::Sector);

    newsScreen->setCurrentFilter(NewsFilter::Corporate);
    EXPECT_EQ(newsScreen->getCurrentFilter(), NewsFilter::Corporate);

    newsScreen->setCurrentFilter(NewsFilter::All);
    EXPECT_EQ(newsScreen->getCurrentFilter(), NewsFilter::All);
}

TEST_F(NewsScreenTest, Pagination) {
    int initialPage = newsScreen->getCurrentPage();
    EXPECT_EQ(initialPage, 0);

    for (int i = 0; i < 20; i++) {
        News testNews(NewsType::Global, "Test News " + std::to_string(i),
                     "Test content " + std::to_string(i), 0.01, 10 + i);
        newsService->addCustomNews(testNews);
    }

    newsScreen->update();

    int totalPages = newsScreen->getTotalPages();
    EXPECT_GT(totalPages, 1);

    newsScreen->setCurrentPage(1);
    EXPECT_EQ(newsScreen->getCurrentPage(), 1);

    newsScreen->setCurrentPage(totalPages - 1);
    EXPECT_EQ(newsScreen->getCurrentPage(), totalPages - 1);

    newsScreen->setCurrentPage(-1);
    EXPECT_EQ(newsScreen->getCurrentPage(), totalPages - 1); // Should not change
}

TEST_F(NewsScreenTest, NewsPerPageSetting) {
    int defaultNewsPerPage = newsScreen->getNewsPerPage();

    newsScreen->setNewsPerPage(10);
    EXPECT_EQ(newsScreen->getNewsPerPage(), 10);

    newsScreen->setNewsPerPage(0);
    EXPECT_EQ(newsScreen->getNewsPerPage(), 10); // Should not change

    newsScreen->setNewsPerPage(defaultNewsPerPage);
}

TEST_F(NewsScreenTest, HandleInput) {
    EXPECT_TRUE(newsScreen->handleInput('6')); // Change filter
    EXPECT_TRUE(newsScreen->handleInput('7')); // Previous page
    EXPECT_TRUE(newsScreen->handleInput('8')); // Next page

    EXPECT_TRUE(newsScreen->handleInput('1')); // View first news

    EXPECT_FALSE(newsScreen->handleInput('0')); // Return to main menu
}

}