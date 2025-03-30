#include <gtest/gtest.h>
#include "../../../src/ui/screens/CompanyScreen.hpp"
#include "../../../src/core/Market.hpp"
#include "../../../src/core/Player.hpp"
#include "../../../src/services/NewsService.hpp"
#include "../../../src/utils/Console.hpp"
#include <memory>

using namespace StockMarketSimulator;

// Test fixture for CompanyScreen tests
class CompanyScreenTest : public ::testing::Test {
protected:
    std::shared_ptr<Market> market;
    std::shared_ptr<Player> player;
    std::shared_ptr<NewsService> newsService;
    std::shared_ptr<CompanyScreen> companyScreen;
    std::shared_ptr<Company> testCompany;

    void SetUp() override {
        // Initialize market with one test company
        market = std::make_shared<Market>();

        testCompany = std::make_shared<Company>(
            "TestCorp", "TEST",
            "A test company for unit testing",
            Sector::Technology,
            100.0, 0.5,
            DividendPolicy(1.0, 4)
        );

        market->addCompany(testCompany);

        // Initialize player
        player = std::make_shared<Player>("TestPlayer", 10000.0);
        player->setMarket(market);

        // Initialize news service
        newsService = std::make_shared<NewsService>(market);

        // Create news about the test company
        News companyNews(
            NewsType::Corporate,
            "Test News Title",
            "This is test news content about TestCorp.",
            0.02, // positive impact
            1,    // day 1
            testCompany
        );

        newsService->addCustomNews(companyNews);

        // Initialize company screen
        companyScreen = std::make_shared<CompanyScreen>();
        companyScreen->setMarket(market);
        companyScreen->setPlayer(player);
        companyScreen->setNewsService(newsService);
        companyScreen->setPosition(0, 0);
        companyScreen->setSize(50, 30);
        companyScreen->initialize();
    }

    void TearDown() override {
        market.reset();
        player.reset();
        newsService.reset();
        companyScreen.reset();
        testCompany.reset();
    }
};

// Test initialization of CompanyScreen
TEST_F(CompanyScreenTest, Initialization) {
    ASSERT_TRUE(companyScreen != nullptr);
    EXPECT_EQ(companyScreen->getTitle(), "Company");

    companyScreen->setCompany(testCompany);
    EXPECT_EQ(companyScreen->getTitle(), "TestCorp (TEST)");
    EXPECT_EQ(companyScreen->getCompany(), testCompany);
}

// Test company getters/setters
TEST_F(CompanyScreenTest, CompanyAccessors) {
    // Initially null
    EXPECT_EQ(companyScreen->getCompany(), nullptr);

    // Set company
    companyScreen->setCompany(testCompany);
    EXPECT_EQ(companyScreen->getCompany(), testCompany);

    // Change company
    auto anotherCompany = std::make_shared<Company>(
        "AnotherCorp", "ANTR",
        "Another test company",
        Sector::Energy,
        50.0, 0.3,
        DividendPolicy(0.5, 2)
    );

    companyScreen->setCompany(anotherCompany);
    EXPECT_EQ(companyScreen->getCompany(), anotherCompany);
    EXPECT_EQ(companyScreen->getTitle(), "AnotherCorp (ANTR)");
}

// Test update method
TEST_F(CompanyScreenTest, Update) {
    companyScreen->setCompany(testCompany);

    // Get original stock price
    double originalPrice = testCompany->getStock()->getCurrentPrice();

    // Change the price
    double newPrice = originalPrice * 1.5;
    testCompany->getStock()->updatePrice(newPrice);

    // Call update to refresh the screen data
    companyScreen->update();

    // Since we can't directly test the display, we'll check that the
    // company reference still points to the same company with updated price
    EXPECT_EQ(companyScreen->getCompany(), testCompany);
    EXPECT_DOUBLE_EQ(companyScreen->getCompany()->getStock()->getCurrentPrice(), newPrice);
}

// Test input handling - Buy action (key '1')
TEST_F(CompanyScreenTest, HandleBuyInput) {
    companyScreen->setCompany(testCompany);

    // Record initial portfolio state
    double initialCash = player->getPortfolio()->getCashBalance();
    int initialShares = player->getPortfolio()->getPositionQuantity("TEST");

    // Simulate input for buy action
    bool shouldContinue = companyScreen->handleInput('1');

    // The screen should continue running after a buy action
    EXPECT_TRUE(shouldContinue);

    // We can't directly test the UI interaction for entering quantities,
    // so we'll test indirectly by triggering a buy through the Player interface

    // Buy 10 shares
    bool buyResult = player->buyStock(testCompany, 10);
    EXPECT_TRUE(buyResult);

    // Verify portfolio changes
    double expectedCost = 10 * testCompany->getStock()->getCurrentPrice() * 1.01; // With 1% commission
    EXPECT_NEAR(player->getPortfolio()->getCashBalance(), initialCash - expectedCost, 0.01);
    EXPECT_EQ(player->getPortfolio()->getPositionQuantity("TEST"), initialShares + 10);
}

// Test input handling - Sell action (key '2')
TEST_F(CompanyScreenTest, HandleSellInput) {
    companyScreen->setCompany(testCompany);

    // First buy some shares so we have something to sell
    player->buyStock(testCompany, 10);

    // Record state after buying
    double cashAfterBuy = player->getPortfolio()->getCashBalance();
    int sharesAfterBuy = player->getPortfolio()->getPositionQuantity("TEST");

    // Simulate input for sell action
    bool shouldContinue = companyScreen->handleInput('2');

    // The screen should continue running after a sell action
    EXPECT_TRUE(shouldContinue);

    // Again, we can't test UI interaction directly, so we'll test the sell functionality

    // Sell 5 shares
    bool sellResult = player->sellStock(testCompany, 5);
    EXPECT_TRUE(sellResult);

    // Verify portfolio changes
    double expectedProceeds = 5 * testCompany->getStock()->getCurrentPrice() * 0.99; // With 1% commission
    EXPECT_NEAR(player->getPortfolio()->getCashBalance(), cashAfterBuy + expectedProceeds, 0.01);
    EXPECT_EQ(player->getPortfolio()->getPositionQuantity("TEST"), sharesAfterBuy - 5);
}

// Test input handling - Return to market (key '3')
TEST_F(CompanyScreenTest, HandleReturnToMarketInput) {
    companyScreen->setCompany(testCompany);

    // Activate the screen
    companyScreen->setActive(true);
    EXPECT_TRUE(companyScreen->isActive());

    // Simulate input for return to market
    bool shouldContinue = companyScreen->handleInput('3');

    // The screen should NOT continue running (return false)
    EXPECT_FALSE(shouldContinue);

    // The screen should be closed (not active)
    EXPECT_FALSE(companyScreen->isActive());
}

// Test input handling - ESC key
TEST_F(CompanyScreenTest, HandleEscapeInput) {
    companyScreen->setCompany(testCompany);

    // Activate the screen
    companyScreen->setActive(true);
    EXPECT_TRUE(companyScreen->isActive());

    // Simulate pressing ESC (ASCII 27)
    bool shouldContinue = companyScreen->handleInput(27);

    // The screen should NOT continue running (return false)
    EXPECT_FALSE(shouldContinue);

    // The screen should be closed (not active)
    EXPECT_FALSE(companyScreen->isActive());
}

// Test with no company set
TEST_F(CompanyScreenTest, NoCompanySet) {
    // Don't set a company
    EXPECT_EQ(companyScreen->getCompany(), nullptr);

    // Make screen active
    companyScreen->setActive(true);

    // Simulate input for buy action
    bool shouldContinue = companyScreen->handleInput('1');

    // Screen should remain active since the buy action should handle the case of no company
    EXPECT_TRUE(shouldContinue);
    EXPECT_TRUE(companyScreen->isActive());
}

// Integration test with NewsService
TEST_F(CompanyScreenTest, NewsServiceIntegration) {
    companyScreen->setCompany(testCompany);

    // Add news about the company
    News testNews(
        NewsType::Corporate,
        "Quarterly Results",
        "TestCorp announces better than expected quarterly results.",
        0.05, // positive impact
        10,   // day 10
        testCompany
    );

    newsService->addCustomNews(testNews);

    // Update the screen to refresh news
    companyScreen->update();

    // Since we can't directly test the UI output, we'll check the NewsService state
    auto allNews = newsService->getNewsHistory();
    bool foundNews = false;

    for (const auto& news : allNews) {
        auto newsCompany = news.getTargetCompany().lock();
        if (newsCompany && newsCompany->getTicker() == "TEST" && news.getTitle() == "Quarterly Results") {
            foundNews = true;
            break;
        }
    }

    EXPECT_TRUE(foundNews) << "TestCorp news should be present in NewsService";
}

// Main function to run tests