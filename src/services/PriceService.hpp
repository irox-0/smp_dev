#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <nlohmann/json.hpp>
#include "../models/Company.hpp"
#include "../core/Market.hpp"
#include "../utils/Random.hpp"
#include "../utils/FileIO.hpp"

namespace StockMarketSimulator {

struct SectorVolatilityProfile {
    double baseVolatility;
    double marketSensitivity;
    double newsSensitivity;
    double cycleSensitivity;

    SectorVolatilityProfile(double base = 0.01, double market = 0.5,
                            double news = 0.8, double cycle = 0.3)
        : baseVolatility(base), marketSensitivity(market),
          newsSensitivity(news), cycleSensitivity(cycle)
    {}

    nlohmann::json toJson() const;
    static SectorVolatilityProfile fromJson(const nlohmann::json& json);
};

struct EconomicCycleParams {
    int cycleLength;
    int currentPosition;
    double amplitude;
    double phaseShift;

    EconomicCycleParams(int length = 365, int position = 0,
                       double amp = 0.02, double shift = 0.0)
        : cycleLength(length), currentPosition(position),
          amplitude(amp), phaseShift(shift)
    {}

    nlohmann::json toJson() const;
    static EconomicCycleParams fromJson(const nlohmann::json& json);
};

class PriceService {
private:
    std::weak_ptr<Market> market;

    double marketVolatilityFactor;
    double trendStrength;
    double momentumFactor;
    double randomnessFactor;

    std::map<Sector, SectorVolatilityProfile> sectorProfiles;
    EconomicCycleParams economicCycle;

    std::map<std::string, std::vector<double>> priceMovementHistory;

    double generateTrendComponent(MarketTrend trend) const;
    double generateCyclicalComponent() const;
    double generateRandomComponent(double volatility) const;
    double generateSectorComponent(Sector sector, MarketTrend trend) const;
    double calculateMomentumEffect(const std::string& ticker, double newMovement);

    void initializeSectorProfiles();

public:
    PriceService();
    PriceService(std::weak_ptr<Market> market);

    void initialize();
    void setMarket(std::weak_ptr<Market> market);

    void updatePrices();
    double generatePriceMovement(const std::shared_ptr<Company>& company, MarketTrend trend);
    void advanceEconomicCycle(int days = 1);

    void simulateMarketShock(double magnitude, bool positive = false);
    void simulateSectorShock(Sector sector, double magnitude, bool positive = false);

    double getMarketVolatilityFactor() const;
    void setMarketVolatilityFactor(double factor);

    double getTrendStrength() const;
    void setTrendStrength(double strength);

    double getMomentumFactor() const;
    void setMomentumFactor(double factor);

    double getRandomnessFactor() const;
    void setRandomnessFactor(double factor);

    const EconomicCycleParams& getEconomicCycleParams() const;
    void setEconomicCycleParams(const EconomicCycleParams& params);

    const SectorVolatilityProfile& getSectorProfile(Sector sector) const;
    void setSectorProfile(Sector sector, const SectorVolatilityProfile& profile);

    const std::vector<double>& getPriceMovementHistory(const std::string& ticker) const;

    nlohmann::json toJson() const;
    static PriceService fromJson(const nlohmann::json& json, std::weak_ptr<Market> market);
};

}