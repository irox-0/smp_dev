#include <gtest/gtest.h>
#include "services/PriceService.hpp"
#include "core/Market.hpp"
#include "models/Company.hpp"
#include "utils/Random.hpp"

using namespace StockMarketSimulator;

class PriceServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        Random::initialize(42);
        
        market = std::make_shared<Market>();
        
        addTestCompanies();
        
        priceService = std::make_unique<PriceService>(market);
        priceService->initialize();
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
        
        market->addCompany(tech1);
        market->addCompany(energy1);
        market->addCompany(finance1);
    }
    
    std::shared_ptr<Market> market;
    std::unique_ptr<PriceService> priceService;
};

TEST_F(PriceServiceTest, InitializationTest) {
    ASSERT_NE(priceService->getMarketVolatilityFactor(), 0.0);
    ASSERT_NE(priceService->getTrendStrength(), 0.0);
    ASSERT_NE(priceService->getMomentumFactor(), 0.0);
    ASSERT_NE(priceService->getRandomnessFactor(), 0.0);
    
    ASSERT_GT(priceService->getSectorProfile(Sector::Technology).baseVolatility, 0.0);
    ASSERT_GT(priceService->getSectorProfile(Sector::Energy).baseVolatility, 0.0);
    ASSERT_GT(priceService->getSectorProfile(Sector::Finance).baseVolatility, 0.0);
    ASSERT_GT(priceService->getSectorProfile(Sector::Consumer).baseVolatility, 0.0);
    ASSERT_GT(priceService->getSectorProfile(Sector::Manufacturing).baseVolatility, 0.0);
    
    const auto& cycleParams = priceService->getEconomicCycleParams();
    ASSERT_GT(cycleParams.cycleLength, 0);
    ASSERT_GE(cycleParams.currentPosition, 0);
    ASSERT_LT(cycleParams.currentPosition, cycleParams.cycleLength);
    ASSERT_GT(cycleParams.amplitude, 0.0);
}

TEST_F(PriceServiceTest, UpdatePricesTest) {
    auto techCompany = market->getCompanyByTicker("TTECH");
    auto energyCompany = market->getCompanyByTicker("TENRG");
    auto financeCompany = market->getCompanyByTicker("TFIN");
    
    ASSERT_NE(techCompany, nullptr);
    ASSERT_NE(energyCompany, nullptr);
    ASSERT_NE(financeCompany, nullptr);
    
    double initialTechPrice = techCompany->getStock()->getCurrentPrice();
    double initialEnergyPrice = energyCompany->getStock()->getCurrentPrice();
    double initialFinancePrice = financeCompany->getStock()->getCurrentPrice();
    
    priceService->updatePrices();
    
    ASSERT_NE(techCompany->getStock()->getCurrentPrice(), initialTechPrice);
    ASSERT_NE(energyCompany->getStock()->getCurrentPrice(), initialEnergyPrice);
    ASSERT_NE(financeCompany->getStock()->getCurrentPrice(), initialFinancePrice);
    
    ASSERT_EQ(priceService->getPriceMovementHistory("TTECH").size(), 1);
    ASSERT_EQ(priceService->getPriceMovementHistory("TENRG").size(), 1);
    ASSERT_EQ(priceService->getPriceMovementHistory("TFIN").size(), 1);
}

TEST_F(PriceServiceTest, GeneratePriceMovementTest) {
    auto techCompany = market->getCompanyByTicker("TTECH");
    auto energyCompany = market->getCompanyByTicker("TENRG");
    
    ASSERT_NE(techCompany, nullptr);
    ASSERT_NE(energyCompany, nullptr);
    
    double bearishTechMovement = priceService->generatePriceMovement(techCompany, MarketTrend::Bearish);
    double bullishTechMovement = priceService->generatePriceMovement(techCompany, MarketTrend::Bullish);
    double bullishEnergyMovement = priceService->generatePriceMovement(energyCompany, MarketTrend::Bullish);
    double bearishEnergyMovement = priceService->generatePriceMovement(energyCompany, MarketTrend::Bearish);
    
    ASSERT_NE(bullishTechMovement, 0.0);
    ASSERT_NE(bearishTechMovement, 0.0);
    ASSERT_NE(bullishEnergyMovement, 0.0);
    ASSERT_NE(bearishEnergyMovement, 0.0);
    
    ASSERT_GT(bullishTechMovement, bearishTechMovement);
    ASSERT_GT(bullishEnergyMovement, bearishEnergyMovement);
    
    ASSERT_EQ(priceService->getPriceMovementHistory("TTECH").size(), 2);
    ASSERT_EQ(priceService->getPriceMovementHistory("TENRG").size(), 2);
}

TEST_F(PriceServiceTest, EconomicCycleTest) {
    const auto& initialCycle = priceService->getEconomicCycleParams();
    int initialPosition = initialCycle.currentPosition;
    
    priceService->advanceEconomicCycle(10);
    
    const auto& updatedCycle = priceService->getEconomicCycleParams();
    int newPosition = updatedCycle.currentPosition;
    
    ASSERT_EQ(newPosition, (initialPosition + 10) % updatedCycle.cycleLength);
    
    priceService->advanceEconomicCycle(updatedCycle.cycleLength);
    const auto& fullCycleParams = priceService->getEconomicCycleParams();
    
    ASSERT_EQ(fullCycleParams.currentPosition, newPosition);
}

TEST_F(PriceServiceTest, MarketShockTest) {
    auto techCompany = market->getCompanyByTicker("TTECH");
    auto energyCompany = market->getCompanyByTicker("TENRG");
    
    ASSERT_NE(techCompany, nullptr);
    ASSERT_NE(energyCompany, nullptr);
    
    double initialTechPrice = techCompany->getStock()->getCurrentPrice();
    double initialEnergyPrice = energyCompany->getStock()->getCurrentPrice();
    double initialMarketIndex = market->getMarketIndex();
    
    priceService->simulateMarketShock(0.05, true);
    
    ASSERT_GT(market->getMarketIndex(), initialMarketIndex);
    ASSERT_GT(techCompany->getStock()->getCurrentPrice(), initialTechPrice);
    ASSERT_GT(energyCompany->getStock()->getCurrentPrice(), initialEnergyPrice);
    
    double midTechPrice = techCompany->getStock()->getCurrentPrice();
    double midEnergyPrice = energyCompany->getStock()->getCurrentPrice();
    double midMarketIndex = market->getMarketIndex();
    
    priceService->simulateMarketShock(0.05, false);
    
    ASSERT_LT(market->getMarketIndex(), midMarketIndex);
    ASSERT_LT(techCompany->getStock()->getCurrentPrice(), midTechPrice);
    ASSERT_LT(energyCompany->getStock()->getCurrentPrice(), midEnergyPrice);
}

TEST_F(PriceServiceTest, SectorShockTest) {
    auto techCompany = market->getCompanyByTicker("TTECH");
    auto energyCompany = market->getCompanyByTicker("TENRG");
    
    ASSERT_NE(techCompany, nullptr);
    ASSERT_NE(energyCompany, nullptr);
    
    double initialTechPrice = techCompany->getStock()->getCurrentPrice();
    double initialEnergyPrice = energyCompany->getStock()->getCurrentPrice();
    
    priceService->simulateSectorShock(Sector::Technology, 0.05, true);
    
    ASSERT_GT(techCompany->getStock()->getCurrentPrice(), initialTechPrice);
    ASSERT_EQ(energyCompany->getStock()->getCurrentPrice(), initialEnergyPrice);
    
    double midTechPrice = techCompany->getStock()->getCurrentPrice();
    
    priceService->simulateSectorShock(Sector::Technology, 0.05, false);
    
    ASSERT_LT(techCompany->getStock()->getCurrentPrice(), midTechPrice);
}

TEST_F(PriceServiceTest, SerializationTest) {
    for (int i = 0; i < 5; i++) {
        priceService->updatePrices();
    }
    
    nlohmann::json json = priceService->toJson();
    
    PriceService restoredService = PriceService::fromJson(json, market);
    
    ASSERT_EQ(restoredService.getMarketVolatilityFactor(), priceService->getMarketVolatilityFactor());
    ASSERT_EQ(restoredService.getTrendStrength(), priceService->getTrendStrength());
    ASSERT_EQ(restoredService.getMomentumFactor(), priceService->getMomentumFactor());
    ASSERT_EQ(restoredService.getRandomnessFactor(), priceService->getRandomnessFactor());
    
    const auto& originalCycle = priceService->getEconomicCycleParams();
    const auto& restoredCycle = restoredService.getEconomicCycleParams();
    
    ASSERT_EQ(restoredCycle.cycleLength, originalCycle.cycleLength);
    ASSERT_EQ(restoredCycle.currentPosition, originalCycle.currentPosition);
    ASSERT_EQ(restoredCycle.amplitude, originalCycle.amplitude);
    ASSERT_EQ(restoredCycle.phaseShift, originalCycle.phaseShift);
    
    for (const auto& ticker : {"TTECH", "TENRG", "TFIN"}) {
        const auto& originalHistory = priceService->getPriceMovementHistory(ticker);
        const auto& restoredHistory = restoredService.getPriceMovementHistory(ticker);
        
        ASSERT_EQ(restoredHistory.size(), originalHistory.size());
        
        for (size_t i = 0; i < originalHistory.size(); i++) {
            ASSERT_NEAR(restoredHistory[i], originalHistory[i], 0.001);
        }
    }
}

TEST_F(PriceServiceTest, SettersTest) {
    priceService->setMarketVolatilityFactor(0.02);
    ASSERT_EQ(priceService->getMarketVolatilityFactor(), 0.02);
    
    priceService->setTrendStrength(0.8);
    ASSERT_EQ(priceService->getTrendStrength(), 0.8);
    
    priceService->setMomentumFactor(0.4);
    ASSERT_EQ(priceService->getMomentumFactor(), 0.4);
    
    priceService->setRandomnessFactor(0.6);
    ASSERT_EQ(priceService->getRandomnessFactor(), 0.6);
    
    priceService->setMarketVolatilityFactor(-0.01);
    ASSERT_NE(priceService->getMarketVolatilityFactor(), -0.01);
    
    priceService->setTrendStrength(1.5);
    ASSERT_NE(priceService->getTrendStrength(), 1.5);
    
    EconomicCycleParams newCycle(500, 100, 0.03, 1.5);
    priceService->setEconomicCycleParams(newCycle);
    
    const auto& updatedCycle = priceService->getEconomicCycleParams();
    ASSERT_EQ(updatedCycle.cycleLength, 500);
    ASSERT_EQ(updatedCycle.currentPosition, 100);
    ASSERT_EQ(updatedCycle.amplitude, 0.03);
    ASSERT_EQ(updatedCycle.phaseShift, 1.5);
    
    SectorVolatilityProfile newProfile(0.03, 0.9, 0.8, 0.7);
    priceService->setSectorProfile(Sector::Technology, newProfile);
    
    const auto& updatedProfile = priceService->getSectorProfile(Sector::Technology);
    ASSERT_EQ(updatedProfile.baseVolatility, 0.03);
    ASSERT_EQ(updatedProfile.marketSensitivity, 0.9);
    ASSERT_EQ(updatedProfile.newsSensitivity, 0.8);
    ASSERT_EQ(updatedProfile.cycleSensitivity, 0.7);
}

TEST_F(PriceServiceTest, MomentumEffectTest) {
    auto techCompany = market->getCompanyByTicker("TTECH");
    ASSERT_NE(techCompany, nullptr);
    
    market->setMarketTrend(MarketTrend::Bullish);

    priceService->setMomentumFactor(0.8);

    for (int i = 0; i < 3; i++) {
        double movement = priceService->generatePriceMovement(techCompany, MarketTrend::Bullish);
        ASSERT_GT(movement, 0.0);
    }

    const auto& history = priceService->getPriceMovementHistory("TTECH");
    ASSERT_GE(history.size(), 3);

    ASSERT_GT(history[history.size() - 1], 0.0);
    ASSERT_GT(history[history.size() - 2], 0.0);
    ASSERT_GT(history[history.size() - 3], 0.0);

    market->setMarketTrend(MarketTrend::Bearish);
    double bearishMovement = priceService->generatePriceMovement(techCompany, MarketTrend::Bearish);

    const auto& updatedHistory = priceService->getPriceMovementHistory("TTECH");
    double lastMovement = updatedHistory[updatedHistory.size() - 1];

    ASSERT_GT(lastMovement, -0.02);
}

TEST_F(PriceServiceTest, LongTermStabilityTest) {
    auto techCompany = market->getCompanyByTicker("TTECH");
    ASSERT_NE(techCompany, nullptr);

    double initialPrice = techCompany->getStock()->getCurrentPrice();
    double minPrice = initialPrice;
    double maxPrice = initialPrice;

    for (int i = 0; i < 100; i++) {
        priceService->updatePrices();
        double currentPrice = techCompany->getStock()->getCurrentPrice();
        minPrice = std::min(minPrice, currentPrice);
        maxPrice = std::max(maxPrice, currentPrice);
    }

    ASSERT_GT(minPrice, initialPrice * 0.4);
    ASSERT_LT(maxPrice, initialPrice * 2.5);

    const auto& history = priceService->getPriceMovementHistory("TTECH");
    ASSERT_LE(history.size(), 100);
}

TEST_F(PriceServiceTest, MultiSectorComparisonTest) {
    auto techCompany = market->getCompanyByTicker("TTECH");
    ASSERT_NE(techCompany, nullptr);

    double initialTechPrice = techCompany->getStock()->getCurrentPrice();
    std::cout << "Initial tech price: " << initialTechPrice << std::endl;

    double targetPrice = initialTechPrice * 1.3;
    std::cout << "Target tech price: " << targetPrice << std::endl;

    market->setMarketTrend(MarketTrend::Bullish);

    for (int i = 0; i < 5; i++) {
        priceService->simulateSectorShock(Sector::Technology, 0.25, true);

        double currentPrice = techCompany->getStock()->getCurrentPrice();
        std::cout << "After shock #" << (i+1) << ": " << currentPrice
                  << " (change: " << ((currentPrice - initialTechPrice) / initialTechPrice * 100.0) << "%)" << std::endl;

        if (currentPrice >= targetPrice) {
            break;
        }
    }

    double finalTechPrice = techCompany->getStock()->getCurrentPrice();
    double techChange = (finalTechPrice - initialTechPrice) / initialTechPrice;

    std::cout << "Final tech price: " << finalTechPrice << std::endl;
    std::cout << "Tech change: " << (techChange * 100.0) << "%" << std::endl;

    ASSERT_GT(techChange, 0.1);
}
TEST_F(PriceServiceTest, WeakPtrTest) {
    PriceService emptyService;
    
    ASSERT_NO_THROW(emptyService.updatePrices());
    ASSERT_NO_THROW(emptyService.simulateMarketShock(0.05, true));
    
    {
        auto tempMarket = std::make_shared<Market>();
        tempMarket->addDefaultCompanies();
        
        emptyService.setMarket(tempMarket);
        
        ASSERT_NO_THROW(emptyService.updatePrices());
    }
    
    ASSERT_NO_THROW(emptyService.updatePrices());
}