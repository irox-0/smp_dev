#include <gtest/gtest.h>
#include "../../src/core/Market.hpp"
#include "../../src/models/Company.hpp"
#include "../../src/utils/Random.hpp"
#include "../../src/utils/Date.hpp"
#include <memory>
#include <string>

namespace StockMarketSimulator {

class MarketTest : public ::testing::Test {
protected:
    std::shared_ptr<Market> market;

    void SetUp() override {
        // Initialize Random utility with fixed seed for consistent test results
        Random::initialize(42);

        // Create a new market for each test
        market = std::make_shared<Market>();
    }

    void TearDown() override {
        market.reset();
    }

    // Helper function to create a test company
    std::shared_ptr<Company> createTestCompany(
        const std::string& name,
        const std::string& ticker,
        Sector sector,
        double initialPrice
    ) {
        return std::make_shared<Company>(
            name, ticker,
            "Test company description",
            sector,
            initialPrice, 0.5,
            DividendPolicy(2.0, 4)
        );
    }
};

// Test market initialization
TEST_F(MarketTest, Initialization) {
    // Check initial market values
    EXPECT_EQ(market->getCurrentDay(), 0);

    // Check initial date (should be March 1, 2023)
    Date expectedDate(1, 3, 2023);
    EXPECT_TRUE(market->getCurrentDate() == expectedDate);

    EXPECT_NEAR(market->getMarketIndex(), 1000.0, 0.001);
    EXPECT_EQ(market->getCurrentTrend(), MarketTrend::Sideways);
    EXPECT_NEAR(market->getInterestRate(), 0.05, 0.001);
    EXPECT_NEAR(market->getInflationRate(), 0.02, 0.001);
    EXPECT_NEAR(market->getUnemploymentRate(), 0.045, 0.001);
    EXPECT_TRUE(market->getCompanies().empty());
}

// Test adding companies to the market
TEST_F(MarketTest, AddCompany) {
    auto company = createTestCompany("Test Co", "TST", Sector::Technology, 100.0);
    market->addCompany(company);

    EXPECT_EQ(market->getCompanies().size(), 1);
    EXPECT_EQ(market->getCompanies()[0]->getName(), "Test Co");
    EXPECT_EQ(market->getCompanies()[0]->getTicker(), "TST");
}

// Test removing companies from the market
TEST_F(MarketTest, RemoveCompany) {
    auto company1 = createTestCompany("Test Co 1", "TST1", Sector::Technology, 100.0);
    auto company2 = createTestCompany("Test Co 2", "TST2", Sector::Energy, 200.0);

    market->addCompany(company1);
    market->addCompany(company2);
    EXPECT_EQ(market->getCompanies().size(), 2);

    market->removeCompany("TST1");
    EXPECT_EQ(market->getCompanies().size(), 1);
    EXPECT_EQ(market->getCompanies()[0]->getTicker(), "TST2");

    market->removeCompany("TST2");
    EXPECT_TRUE(market->getCompanies().empty());
}

// Test getting company by ticker
TEST_F(MarketTest, GetCompanyByTicker) {
    auto company1 = createTestCompany("Test Co 1", "TST1", Sector::Technology, 100.0);
    auto company2 = createTestCompany("Test Co 2", "TST2", Sector::Energy, 200.0);

    market->addCompany(company1);
    market->addCompany(company2);

    auto foundCompany = market->getCompanyByTicker("TST1");
    EXPECT_NE(foundCompany, nullptr);
    EXPECT_EQ(foundCompany->getName(), "Test Co 1");

    foundCompany = market->getCompanyByTicker("TST2");
    EXPECT_NE(foundCompany, nullptr);
    EXPECT_EQ(foundCompany->getName(), "Test Co 2");

    foundCompany = market->getCompanyByTicker("NONEXISTENT");
    EXPECT_EQ(foundCompany, nullptr);
}

// Test getting companies by sector
TEST_F(MarketTest, GetCompaniesBySector) {
    auto company1 = createTestCompany("Tech 1", "TCH1", Sector::Technology, 100.0);
    auto company2 = createTestCompany("Tech 2", "TCH2", Sector::Technology, 150.0);
    auto company3 = createTestCompany("Energy", "ENRG", Sector::Energy, 200.0);

    market->addCompany(company1);
    market->addCompany(company2);
    market->addCompany(company3);

    auto techCompanies = market->getCompaniesBySector(Sector::Technology);
    EXPECT_EQ(techCompanies.size(), 2);

    auto energyCompanies = market->getCompaniesBySector(Sector::Energy);
    EXPECT_EQ(energyCompanies.size(), 1);
    EXPECT_EQ(energyCompanies[0]->getTicker(), "ENRG");

    auto financeCompanies = market->getCompaniesBySector(Sector::Finance);
    EXPECT_TRUE(financeCompanies.empty());
}

// Test adding default companies
TEST_F(MarketTest, AddDefaultCompanies) {
    market->addDefaultCompanies();

    EXPECT_FALSE(market->getCompanies().empty());
    EXPECT_GE(market->getCompanies().size(), 8); // TS mentions 8-10 companies

    // Verify we have companies in each sector
    EXPECT_FALSE(market->getCompaniesBySector(Sector::Technology).empty());
    EXPECT_FALSE(market->getCompaniesBySector(Sector::Energy).empty());
    EXPECT_FALSE(market->getCompaniesBySector(Sector::Finance).empty());
    EXPECT_FALSE(market->getCompaniesBySector(Sector::Consumer).empty());
    EXPECT_FALSE(market->getCompaniesBySector(Sector::Manufacturing).empty());

    // Verify at least one tech company exists as specified
    bool techCorpFound = false;
    for (const auto& company : market->getCompanies()) {
        if (company->getTicker() == "TCH") {
            techCorpFound = true;
            break;
        }
    }
    EXPECT_TRUE(techCorpFound);
}

// Test market simulation (day advancement)
TEST_F(MarketTest, SimulateDay) {
    market->addDefaultCompanies();
    int initialDay = market->getCurrentDay();
    Date initialDate = market->getCurrentDate();
    double initialIndex = market->getMarketIndex();

    market->simulateDay();

    // Check that day advanced
    EXPECT_EQ(market->getCurrentDay(), initialDay + 1);

    // Check that date advanced by one day
    Date expectedNextDate = initialDate;
    expectedNextDate.nextDay();
    EXPECT_TRUE(market->getCurrentDate() == expectedNextDate);

    // Market index should change (though we can't predict how)
    EXPECT_NE(market->getMarketIndex(), initialIndex);

    // Companies should have updated prices
    for (const auto& company : market->getCompanies()) {
        // The day change should be non-zero since prices got updated
        EXPECT_NE(company->getStock()->getDayChangeAmount(), 0.0);
    }
}

// Test multiple day simulation
TEST_F(MarketTest, SimulateMultipleDays) {
    market->addDefaultCompanies();
    int initialDay = market->getCurrentDay();
    Date initialDate = market->getCurrentDate();

    // Simulate 30 days
    for (int i = 0; i < 30; i++) {
        market->simulateDay();
    }

    EXPECT_EQ(market->getCurrentDay(), initialDay + 30);

    // Check date advanced by 30 days
    Date expectedDate = initialDate;
    expectedDate.advanceDays(30);
    EXPECT_TRUE(market->getCurrentDate() == expectedDate);

    // After 30 days, stock prices should have significant history
    for (const auto& company : market->getCompanies()) {
        EXPECT_GE(company->getStock()->getPriceHistoryLength(), 30);
    }
}

// Test market trend setter
TEST_F(MarketTest, SetMarketTrend) {
    // Set trend to Bullish
    market->setMarketTrend(MarketTrend::Bullish);
    EXPECT_EQ(market->getCurrentTrend(), MarketTrend::Bullish);
    EXPECT_EQ(market->getTrendName(), "Bullish");

    // Set trend to Bearish
    market->setMarketTrend(MarketTrend::Bearish);
    EXPECT_EQ(market->getCurrentTrend(), MarketTrend::Bearish);
    EXPECT_EQ(market->getTrendName(), "Bearish");

    // Set trend to Volatile
    market->setMarketTrend(MarketTrend::Volatile);
    EXPECT_EQ(market->getCurrentTrend(), MarketTrend::Volatile);
    EXPECT_EQ(market->getTrendName(), "Volatile");
}

// Test economic event triggering
TEST_F(MarketTest, TriggerEconomicEvent) {
    market->addDefaultCompanies();
    double initialIndex = market->getMarketIndex();

    // Trigger a positive economic event
    market->triggerEconomicEvent(0.05, true);

    // Market index should increase
    EXPECT_GT(market->getMarketIndex(), initialIndex);

    // All company prices should be affected
    for (const auto& company : market->getCompanies()) {
        // Prices have changed due to the event
        EXPECT_NE(company->getStock()->getCurrentPrice(), company->getStock()->getOpenPrice());
    }

    // Now trigger a negative economic event
    double currentIndex = market->getMarketIndex();
    market->triggerEconomicEvent(-0.05, true);

    // Market index should decrease
    EXPECT_LT(market->getMarketIndex(), currentIndex);
}

// Test economic event affecting specific sectors
TEST_F(MarketTest, TriggerSectorSpecificEvent) {
    market->addDefaultCompanies();

    // Get initial prices for companies in each sector
    std::map<Sector, std::vector<double>> initialSectorPrices;

    for (const auto& company : market->getCompanies()) {
        Sector sector = company->getSector();
        initialSectorPrices[sector].push_back(company->getStock()->getCurrentPrice());
    }

    // Trigger an economic event with affectAllSectors=false
    // This will randomly impact one sector more than others
    market->triggerEconomicEvent(0.1, false);

    // At least some prices should have changed
    bool pricesChanged = false;
    for (const auto& company : market->getCompanies()) {
        if (company->getStock()->getCurrentPrice() != company->getStock()->getOpenPrice()) {
            pricesChanged = true;
            break;
        }
    }

    EXPECT_TRUE(pricesChanged);
}

// Test serialization to JSON
TEST_F(MarketTest, SerializeToJson) {
    market->addDefaultCompanies();
    market->simulateDay(); // To generate some data

    // Convert to JSON
    nlohmann::json marketJson = market->toJson();

    // Verify JSON structure
    EXPECT_TRUE(marketJson.contains("current_date"));
    EXPECT_TRUE(marketJson.contains("market_state"));
    EXPECT_TRUE(marketJson.contains("sector_trends"));
    EXPECT_TRUE(marketJson.contains("companies"));

    // Verify companies are serialized
    EXPECT_EQ(marketJson["companies"].size(), market->getCompanies().size());

    // Verify date is serialized
    EXPECT_TRUE(marketJson["current_date"].contains("day"));
    EXPECT_TRUE(marketJson["current_date"].contains("month"));
    EXPECT_TRUE(marketJson["current_date"].contains("year"));
}

// Test deserialization from JSON
TEST_F(MarketTest, DeserializeFromJson) {
    market->addDefaultCompanies();
    market->simulateDay(); // To generate some data

    Date originalDate = market->getCurrentDate();
    double originalIndex = market->getMarketIndex();
    MarketTrend originalTrend = market->getCurrentTrend();
    size_t originalCompanyCount = market->getCompanies().size();

    // Convert to JSON
    nlohmann::json marketJson = market->toJson();

    // Create a new market from JSON
    Market deserializedMarket = Market::fromJson(marketJson);

    // Verify deserialized market has same properties
    EXPECT_TRUE(deserializedMarket.getCurrentDate() == originalDate);
    EXPECT_NEAR(deserializedMarket.getMarketIndex(), originalIndex, 0.001);
    EXPECT_EQ(deserializedMarket.getCompanies().size(), originalCompanyCount);
    EXPECT_EQ(deserializedMarket.getCurrentTrend(), originalTrend);
}

// Test market trend string conversion
TEST_F(MarketTest, MarketTrendStringConversion) {
    EXPECT_EQ(Market::marketTrendToString(MarketTrend::Bullish), "Bullish");
    EXPECT_EQ(Market::marketTrendToString(MarketTrend::Bearish), "Bearish");
    EXPECT_EQ(Market::marketTrendToString(MarketTrend::Sideways), "Sideways");
    EXPECT_EQ(Market::marketTrendToString(MarketTrend::Volatile), "Volatile");

    EXPECT_EQ(Market::marketTrendFromString("Bullish"), MarketTrend::Bullish);
    EXPECT_EQ(Market::marketTrendFromString("Bearish"), MarketTrend::Bearish);
    EXPECT_EQ(Market::marketTrendFromString("Sideways"), MarketTrend::Sideways);
    EXPECT_EQ(Market::marketTrendFromString("Volatile"), MarketTrend::Volatile);
    EXPECT_EQ(Market::marketTrendFromString("Unknown"), MarketTrend::Sideways); // Default
}

// Test sector string conversion
TEST_F(MarketTest, SectorStringConversion) {
    EXPECT_EQ(Market::sectorToString(Sector::Technology), "Technology");
    EXPECT_EQ(Market::sectorToString(Sector::Energy), "Energy");
    EXPECT_EQ(Market::sectorToString(Sector::Finance), "Finance");
    EXPECT_EQ(Market::sectorToString(Sector::Consumer), "Consumer");
    EXPECT_EQ(Market::sectorToString(Sector::Manufacturing), "Manufacturing");
    EXPECT_EQ(Market::sectorToString(Sector::Unknown), "Unknown");

    EXPECT_EQ(Market::sectorFromString("Technology"), Sector::Technology);
    EXPECT_EQ(Market::sectorFromString("Energy"), Sector::Energy);
    EXPECT_EQ(Market::sectorFromString("Finance"), Sector::Finance);
    EXPECT_EQ(Market::sectorFromString("Consumer"), Sector::Consumer);
    EXPECT_EQ(Market::sectorFromString("Manufacturing"), Sector::Manufacturing);
    EXPECT_EQ(Market::sectorFromString("Unknown"), Sector::Unknown);
}

// Test process company dividends
TEST_F(MarketTest, ProcessCompanyDividends) {
    auto company = createTestCompany("Dividend Co", "DIV", Sector::Finance, 100.0);

    // Setup company with dividend policy
    DividendPolicy policy(4.0, 4); // 4.0 annual rate, quarterly payments
    company->setDividendPolicy(policy);
    market->addCompany(company);

    // Initially dividend shouldn't be due
    company->processDividends(market->getCurrentDate());

    // Advance days to trigger dividend
    for (int i = 0; i < 90; i++) { // Quarterly = ~90 days
        market->simulateDay();
    }

    // Process dividends
    market->processCompanyDividends();

    // Difficult to assert actual dividend payment without a player
    // But we can verify the code executes without errors
}

// Test macroeconomic factors
TEST_F(MarketTest, MacroeconomicFactors) {
    double initialInterestRate = market->getInterestRate();
    double initialInflationRate = market->getInflationRate();
    double initialUnemploymentRate = market->getUnemploymentRate();

    // Simulate several days to let macroeconomic factors change
    for (int i = 0; i < 20; i++) {
        market->simulateDay();
    }

    // Values should eventually change due to random factors
    // We can't predict exact values but they should be within reasonable bounds
    EXPECT_GE(market->getInterestRate(), 0.01);
    EXPECT_LE(market->getInterestRate(), 0.15);

    EXPECT_GE(market->getInflationRate(), 0.0);
    EXPECT_LE(market->getInflationRate(), 0.2);

    EXPECT_GE(market->getUnemploymentRate(), 0.02);
    EXPECT_LE(market->getUnemploymentRate(), 0.15);
}

// Test sector trends
TEST_F(MarketTest, SectorTrends) {
    market->addDefaultCompanies();

    // Initially all sector trends should be 0.0
    const auto& initialSectorTrends = market->getSectorTrends();
    for (const auto& [sector, trend] : initialSectorTrends) {
        EXPECT_NEAR(trend, 0.0, 0.001);
    }

    // Simulate several days to let sector trends evolve
    for (int i = 0; i < 5; i++) {
        market->simulateDay();
    }

    // Now trends should have changed
    const auto& updatedSectorTrends = market->getSectorTrends();
    bool trendsChanged = false;

    for (const auto& [sector, trend] : updatedSectorTrends) {
        if (std::abs(trend) > 0.001) {
            trendsChanged = true;
            break;
        }
    }

    EXPECT_TRUE(trendsChanged);
}

// Test Date-based functionality
TEST_F(MarketTest, DateFunctionality) {
    // Test initial date
    Date initialDate = market->getCurrentDate();
    EXPECT_EQ(initialDate.getDay(), 1);
    EXPECT_EQ(initialDate.getMonth(), 3);
    EXPECT_EQ(initialDate.getYear(), 2023);

    // Simulate a day and check date advances
    market->simulateDay();
    Date nextDate = market->getCurrentDate();

    // Date should have advanced by one day
    int daysBetween = initialDate.daysBetween(nextDate);
    EXPECT_EQ(daysBetween, 1);

    // Check correct month transition
    market->setMarketTrend(MarketTrend::Sideways); // Set stable trend for testing

    // Advance to end of March
    while (market->getCurrentDate().getDay() < 31 ||
           market->getCurrentDate().getMonth() != 3) {
        market->simulateDay();
    }

    // One more day should advance to April 1
    market->simulateDay();
    Date aprilFirst = market->getCurrentDate();

    EXPECT_EQ(aprilFirst.getDay(), 1);
    EXPECT_EQ(aprilFirst.getMonth(), 4);
    EXPECT_EQ(aprilFirst.getYear(), 2023);
}

} // namespace StockMarketSimulator