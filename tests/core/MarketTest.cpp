#include <gtest/gtest.h>
#include "core/Market.hpp"
#include "models/Company.hpp"
#include "utils/Random.hpp"

using namespace StockMarketSimulator;

class MarketTest : public ::testing::Test {
protected:
    void SetUp() override {
        Random::initialize(42);

        testMarket = std::make_unique<Market>();

        addTestCompanies();
    }

    void addTestCompanies() {
        auto tech1 = std::make_shared<Company>(
            "Test Tech", "TTECH",
            "Test technology company", Sector::Technology,
            100.0, 0.5, DividendPolicy(2.0, 4)
        );

        auto energy1 = std::make_shared<Company>(
            "Test Energy", "TENRG",
            "Test energy company", Sector::Energy,
            50.0, 0.4, DividendPolicy(3.0, 4)
        );

        auto finance1 = std::make_shared<Company>(
            "Test Finance", "TFIN",
            "Test finance company", Sector::Finance,
            75.0, 0.3, DividendPolicy(4.0, 4)
        );

        testMarket->addCompany(tech1);
        testMarket->addCompany(energy1);
        testMarket->addCompany(finance1);
    }

    std::unique_ptr<Market> testMarket;
};

TEST_F(MarketTest, InitializationTest) {
    ASSERT_EQ(testMarket->getCurrentDay(), 0);
    ASSERT_GT(testMarket->getMarketIndex(), 0.0);
    ASSERT_EQ(testMarket->getCurrentTrend(), MarketTrend::Sideways);

    const auto& companies = testMarket->getCompanies();
    ASSERT_EQ(companies.size(), 3);
}

TEST_F(MarketTest, CompanyManagementTest) {
    auto company = testMarket->getCompanyByTicker("TTECH");
    ASSERT_NE(company, nullptr);
    ASSERT_EQ(company->getTicker(), "TTECH");

    auto nonExistentCompany = testMarket->getCompanyByTicker("NONE");
    ASSERT_EQ(nonExistentCompany, nullptr);

    auto techCompanies = testMarket->getCompaniesBySector(Sector::Technology);
    ASSERT_EQ(techCompanies.size(), 1);
    ASSERT_EQ(techCompanies[0]->getSector(), Sector::Technology);

    testMarket->removeCompany("TTECH");
    ASSERT_EQ(testMarket->getCompanies().size(), 2);
    ASSERT_EQ(testMarket->getCompanyByTicker("TTECH"), nullptr);
}

TEST_F(MarketTest, DefaultCompaniesTest) {
    Market market;
    ASSERT_EQ(market.getCompanies().size(), 0);

    market.addDefaultCompanies();
    ASSERT_GT(market.getCompanies().size(), 8);

    bool hasTech = false, hasEnergy = false, hasFinance = false,
         hasConsumer = false, hasManufacturing = false;

    for (const auto& company : market.getCompanies()) {
        switch (company->getSector()) {
            case Sector::Technology: hasTech = true; break;
            case Sector::Energy: hasEnergy = true; break;
            case Sector::Finance: hasFinance = true; break;
            case Sector::Consumer: hasConsumer = true; break;
            case Sector::Manufacturing: hasManufacturing = true; break;
            default: break;
        }
    }

    ASSERT_TRUE(hasTech);
    ASSERT_TRUE(hasEnergy);
    ASSERT_TRUE(hasFinance);
    ASSERT_TRUE(hasConsumer);
    ASSERT_TRUE(hasManufacturing);
}

TEST_F(MarketTest, SimulateDayTest) {
    double initialIndex = testMarket->getMarketIndex();
    double initialPrice = testMarket->getCompanyByTicker("TTECH")->getStock()->getCurrentPrice();

    testMarket->simulateDay();

    ASSERT_EQ(testMarket->getCurrentDay(), 1);

    double newIndex = testMarket->getMarketIndex();
    ASSERT_NE(newIndex, initialIndex);

    const auto& state = testMarket->getState();
    ASSERT_NE(state.dailyChange, 0.0);
    ASSERT_NE(state.dailyChangePercent, 0.0);

    double newPrice = testMarket->getCompanyByTicker("TTECH")->getStock()->getCurrentPrice();
    ASSERT_NE(newPrice, initialPrice);
}

TEST_F(MarketTest, MarketTrendTest) {
    testMarket->setMarketTrend(MarketTrend::Bullish);
    ASSERT_EQ(testMarket->getCurrentTrend(), MarketTrend::Bullish);
    ASSERT_EQ(testMarket->getTrendName(), "Bullish");

    double initialIndex = testMarket->getMarketIndex();

    for (int i = 0; i < 10; i++) {
        testMarket->simulateDay();
    }

    ASSERT_GT(testMarket->getMarketIndex(), initialIndex);

    testMarket->setMarketTrend(MarketTrend::Bearish);
    ASSERT_EQ(testMarket->getCurrentTrend(), MarketTrend::Bearish);

    double indexAfterBullish = testMarket->getMarketIndex();

    for (int i = 0; i < 10; i++) {
        testMarket->simulateDay();
    }

    ASSERT_LT(testMarket->getMarketIndex(), indexAfterBullish);
}

TEST_F(MarketTest, EconomicEventTest) {
    double initialIndex = testMarket->getMarketIndex();

    testMarket->triggerEconomicEvent(0.05);

    ASSERT_GT(testMarket->getMarketIndex(), initialIndex);

    double currentIndex = testMarket->getMarketIndex();
    testMarket->triggerEconomicEvent(-0.1);

    ASSERT_LT(testMarket->getMarketIndex(), currentIndex);
}

TEST_F(MarketTest, DividendProcessingTest) {
    auto company = testMarket->getCompanyByTicker("TTECH");
    DividendPolicy policy = company->getDividendPolicy();
    policy.nextPaymentDay = 1; // Выплата на 1-й день
    company->setDividendPolicy(policy);

    testMarket->simulateDay(); // Day 1
    testMarket->processCompanyDividends();

    ASSERT_EQ(company->getDividendPolicy().nextPaymentDay, 1 + 91);
}

TEST_F(MarketTest, MacroeconomicFactorsTest) {
    ASSERT_GT(testMarket->getInterestRate(), 0.0);
    ASSERT_GT(testMarket->getInflationRate(), 0.0);
    ASSERT_GT(testMarket->getUnemploymentRate(), 0.0);

    for (int i = 0; i < 100; i++) {
        testMarket->simulateDay();
    }

    ASSERT_GE(testMarket->getInterestRate(), 0.01);
    ASSERT_LE(testMarket->getInterestRate(), 0.15);

    ASSERT_GE(testMarket->getInflationRate(), 0.0);
    ASSERT_LE(testMarket->getInflationRate(), 0.2);

    ASSERT_GE(testMarket->getUnemploymentRate(), 0.02);
    ASSERT_LE(testMarket->getUnemploymentRate(), 0.15);
}

TEST_F(MarketTest, JsonSerializationTest) {
    for (int i = 0; i < 5; i++) {
        testMarket->simulateDay();
    }

    nlohmann::json json = testMarket->toJson();

    Market restoredMarket = Market::fromJson(json);

    ASSERT_EQ(restoredMarket.getCurrentDay(), testMarket->getCurrentDay());
    ASSERT_EQ(restoredMarket.getMarketIndex(), testMarket->getMarketIndex());
    ASSERT_EQ(restoredMarket.getCurrentTrend(), testMarket->getCurrentTrend());
    ASSERT_EQ(restoredMarket.getCompanies().size(), testMarket->getCompanies().size());

    ASSERT_EQ(restoredMarket.getInterestRate(), testMarket->getInterestRate());
    ASSERT_EQ(restoredMarket.getInflationRate(), testMarket->getInflationRate());
    ASSERT_EQ(restoredMarket.getUnemploymentRate(), testMarket->getUnemploymentRate());
}

TEST_F(MarketTest, SectorTrendsTest) {
    const auto& sectorTrends = testMarket->getSectorTrends();
    ASSERT_FALSE(sectorTrends.empty());

    ASSERT_TRUE(sectorTrends.find(Sector::Technology) != sectorTrends.end());
    ASSERT_TRUE(sectorTrends.find(Sector::Energy) != sectorTrends.end());
    ASSERT_TRUE(sectorTrends.find(Sector::Finance) != sectorTrends.end());

    EXPECT_EQ(Market::sectorToString(Sector::Technology), "Technology");
    EXPECT_EQ(Market::sectorToString(Sector::Energy), "Energy");
    EXPECT_EQ(Market::sectorToString(Sector::Finance), "Finance");
    EXPECT_EQ(Market::sectorToString(Sector::Consumer), "Consumer");
    EXPECT_EQ(Market::sectorToString(Sector::Manufacturing), "Manufacturing");

    EXPECT_EQ(Market::sectorFromString("Technology"), Sector::Technology);
    EXPECT_EQ(Market::sectorFromString("Energy"), Sector::Energy);
    EXPECT_EQ(Market::sectorFromString("Finance"), Sector::Finance);
    EXPECT_EQ(Market::sectorFromString("Consumer"), Sector::Consumer);
    EXPECT_EQ(Market::sectorFromString("Manufacturing"), Sector::Manufacturing);
    EXPECT_EQ(Market::sectorFromString("Unknown"), Sector::Unknown);

    ASSERT_NO_THROW(testMarket->simulateDay());
}

TEST_F(MarketTest, MarketTrendConversionTest) {
    ASSERT_EQ(Market::marketTrendFromString(Market::marketTrendToString(MarketTrend::Bullish)), MarketTrend::Bullish);
    ASSERT_EQ(Market::marketTrendFromString(Market::marketTrendToString(MarketTrend::Bearish)), MarketTrend::Bearish);
    ASSERT_EQ(Market::marketTrendFromString(Market::marketTrendToString(MarketTrend::Sideways)), MarketTrend::Sideways);
    ASSERT_EQ(Market::marketTrendFromString(Market::marketTrendToString(MarketTrend::Volatile)), MarketTrend::Volatile);

    ASSERT_EQ(Market::marketTrendFromString("Unknown"), MarketTrend::Sideways);
}