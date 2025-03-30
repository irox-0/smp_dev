#include <gtest/gtest.h>
#include "../../../src/ui/screens/MarketScreen.hpp"
#include "../../../src/ui/screens/CompanyScreen.hpp"
#include "../../../src/core/Market.hpp"
#include "../../../src/core/Player.hpp"
#include "../../../src/services/NewsService.hpp"
#include "../../../src/utils/Console.hpp"
#include <memory>

using namespace StockMarketSimulator;

// Integration test fixture for MarketScreen and CompanyScreen
class MarketCompanyIntegrationTest : public ::testing::Test {
protected:
    std::shared_ptr<Market> market;
    std::shared_ptr<Player> player;
    std::shared_ptr<NewsService> newsService;
    std::shared_ptr<MarketScreen> marketScreen;

    void SetUp() override {
        // Initialize market with test companies
        market = std::make_shared<Market>();
        market->addDefaultCompanies();

        // Add a specific test company that we'll use for testing
        auto testCompany = std::make_shared<Company>(
            "IntegrationTest", "ITST",
            "A test company for integration testing",
            Sector::Technology,
            150.0, 0.6,
            DividendPolicy(2.0, 4)
        );
        market->addCompany(testCompany);

        // Initialize player
        player = std::make_shared<Player>("TestPlayer", 20000.0);
        player->setMarket(market);

        // Initialize news service
        newsService = std::make_shared<NewsService>(market);

        // Initialize market screen
        marketScreen = std::make_shared<MarketScreen>();
        marketScreen->setMarket(market);
        marketScreen->setPlayer(player);
        marketScreen->setPosition(0, 0);
        marketScreen->setSize(60, 40);
        marketScreen->initialize();

        // Simulate a few days for price history
        for (int i = 0; i < 10; i++) {
            market->simulateDay();
        }
    }

    void TearDown() override {
        market.reset();
        player.reset();
        newsService.reset();
        marketScreen.reset();
    }

    // Helper to find a company in the market by name
    int findCompanyIndex(const std::string& name) {
        auto companies = market->getCompanies();
        for (size_t i = 0; i < companies.size(); i++) {
            if (companies[i]->getName() == name) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
};

// Test integration through the Player interface
TEST_F(MarketCompanyIntegrationTest, TradingIntegration) {
    int companyIndex = findCompanyIndex("IntegrationTest");
    ASSERT_GE(companyIndex, 0) << "Test company not found in market";

    auto companies = market->getCompanies();
    auto company = companies[companyIndex];

    // Initial cash and portfolio state
    double initialCash = player->getPortfolio()->getCashBalance();
    int initialPositionQuantity = player->getPortfolio()->getPositionQuantity(company->getTicker());

    // Buy some stocks
    int buyQuantity = 10;
    bool buyResult = player->buyStock(company, buyQuantity);
    EXPECT_TRUE(buyResult) << "Failed to buy stocks";

    // Check new position
    double stockPrice = company->getStock()->getCurrentPrice();
    double expectedCost = buyQuantity * stockPrice * 1.01; // With 1% commission

    int newPositionQuantity = player->getPortfolio()->getPositionQuantity(company->getTicker());
    double newCash = player->getPortfolio()->getCashBalance();

    EXPECT_EQ(newPositionQuantity, initialPositionQuantity + buyQuantity)
        << "Company position quantity not updated correctly after buy";
    EXPECT_NEAR(newCash, initialCash - expectedCost, 0.01)
        << "Cash balance not updated correctly after buy";

    // Now sell fewer stocks
    int sellQuantity = 5;
    double preSellCash = newCash;
    bool sellResult = player->sellStock(company, sellQuantity);
    EXPECT_TRUE(sellResult) << "Failed to sell stocks";

    // Check position reduced
    double expectedProceeds = sellQuantity * stockPrice * 0.99; // With 1% commission

    int finalPositionQuantity = player->getPortfolio()->getPositionQuantity(company->getTicker());
    double finalCash = player->getPortfolio()->getCashBalance();

    EXPECT_EQ(finalPositionQuantity, newPositionQuantity - sellQuantity)
        << "Company position quantity not updated correctly after sell";
    EXPECT_NEAR(finalCash, preSellCash + expectedProceeds, 0.01)
        << "Cash balance not updated correctly after sell";
}

// Test market data consistency
TEST_F(MarketCompanyIntegrationTest, MarketDataConsistency) {
    // Find our test company
    int companyIndex = findCompanyIndex("IntegrationTest");
    ASSERT_GE(companyIndex, 0) << "Test company not found in market";

    auto companies = market->getCompanies();
    auto company = companies[companyIndex];

    // Create CompanyScreen
    auto companyScreen = std::make_shared<CompanyScreen>();
    companyScreen->setMarket(market);
    companyScreen->setPlayer(player);
    companyScreen->setCompany(company);
    companyScreen->initialize();

    // Update both screens
    marketScreen->update();
    companyScreen->update();

    // Verify the company data matches
    auto marketCompany = companies[companyIndex];
    auto companyScreenCompany = companyScreen->getCompany();

    ASSERT_TRUE(marketCompany != nullptr);
    ASSERT_TRUE(companyScreenCompany != nullptr);

    EXPECT_EQ(marketCompany->getTicker(), companyScreenCompany->getTicker());
    EXPECT_EQ(marketCompany->getName(), companyScreenCompany->getName());

    // Price should exactly match
    EXPECT_DOUBLE_EQ(
        marketCompany->getStock()->getCurrentPrice(),
        companyScreenCompany->getStock()->getCurrentPrice()
    );

    // Simulate a market change
    market->simulateDay();

    // Update both screens
    marketScreen->update();
    companyScreen->update();

    // Verify both screens still show the same data
    EXPECT_DOUBLE_EQ(
        marketCompany->getStock()->getCurrentPrice(),
        companyScreenCompany->getStock()->getCurrentPrice()
    );
}

// Test navigation between screens using input handling
TEST_F(MarketCompanyIntegrationTest, ScreenNavigation) {
    // Find our test company
    int companyIndex = findCompanyIndex("IntegrationTest");
    ASSERT_GE(companyIndex, 0) << "Test company not found in market";

    // Set market screen as active
    marketScreen->setActive(true);
    EXPECT_TRUE(marketScreen->isActive());

    // When CompanyScreen is instantiated by MarketScreen, we can't directly
    // test it because it's internal to the viewCompanyDetails method.
    // But we can verify that pressing the return key in CompanyScreen
    // would make it inactive.

    // Create a CompanyScreen to test the navigation sequence
    auto companyScreen = std::make_shared<CompanyScreen>();
    companyScreen->setMarket(market);
    companyScreen->setPlayer(player);
    companyScreen->setCompany(market->getCompanies()[companyIndex]);
    companyScreen->initialize();

    // Activate company screen
    companyScreen->setActive(true);
    EXPECT_TRUE(companyScreen->isActive());

    // Simulate pressing 3 (return to market)
    bool continueRunning = companyScreen->handleInput('3');

    // Verify the company screen is now inactive
    EXPECT_FALSE(continueRunning);
    EXPECT_FALSE(companyScreen->isActive());
}

// Test that MarketScreen and CompanyScreen handle player data consistently
TEST_F(MarketCompanyIntegrationTest, PlayerDataConsistency) {
    int companyIndex = findCompanyIndex("IntegrationTest");
    ASSERT_GE(companyIndex, 0) << "Test company not found in market";

    auto company = market->getCompanies()[companyIndex];

    // Create CompanyScreen
    auto companyScreen = std::make_shared<CompanyScreen>();
    companyScreen->setMarket(market);
    companyScreen->setPlayer(player);
    companyScreen->setCompany(company);
    companyScreen->initialize();

    // Execute a trade through the player interface
    double initialCash = player->getPortfolio()->getCashBalance();
    player->buyStock(company, 5);

    // Update both screens
    marketScreen->update();
    companyScreen->update();

    // Verify the player data is reflected in the market - we can't directly
    // check the UI, but the underlying player data should be changed
    EXPECT_LT(player->getPortfolio()->getCashBalance(), initialCash);
    EXPECT_GT(player->getPortfolio()->getPositionQuantity(company->getTicker()), 0);

    // The UI would reflect this data in both screens
}
