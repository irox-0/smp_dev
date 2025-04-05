#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <nlohmann/json.hpp>
#include "../models/Company.hpp"
#include "../utils/Date.hpp"

namespace StockMarketSimulator {

enum class MarketTrend {
    Bullish,
    Bearish,
    Sideways,
    Volatile
};

struct MarketState {
    double indexValue;
    double dailyChange;
    double dailyChangePercent;
    MarketTrend currentTrend;
    int trendDuration;
    double interestRate;
    double inflationRate;
    double unemploymentRate;
};

class Market {
private:
    std::vector<std::shared_ptr<Company>> companies;
    MarketState state;
    std::map<Sector, double> sectorTrends;
    Date currentDate;
    int cycleLength;
    int currentCycleDay;

    double marketVolatility;
    double marketMomentum;
    double trendStrength;

    void updateMarketIndex();
    void updateSectorTrends();
    void calculateMarketTrend();
    void updateMacroeconomicFactors();
    double generateMarketMovement();
    double generateSectorMovement(Sector sector);

public:
    Market();

    const MarketState& getState() const;
    const std::vector<std::shared_ptr<Company>>& getCompanies() const;
    const std::map<Sector, double>& getSectorTrends() const;
    Date getCurrentDate() const;
    int getCurrentDay() const;
    double getMarketIndex() const;
    MarketTrend getCurrentTrend() const;
    std::string getTrendName() const;
    double getInterestRate() const;
    double getInflationRate() const;
    double getUnemploymentRate() const;

    void addCompany(std::shared_ptr<Company> company);
    void removeCompany(const std::string& ticker);
    std::shared_ptr<Company> getCompanyByTicker(const std::string& ticker) const;
    std::vector<std::shared_ptr<Company>> getCompaniesBySector(Sector sector) const;
    void addDefaultCompanies();

    void simulateDay();
    void processCompanyDividends();
    void setMarketTrend(MarketTrend trend);
    void triggerEconomicEvent(double impact, bool affectAllSectors = true);

    nlohmann::json toJson() const;
    static Market fromJson(const nlohmann::json& json);

    static std::string marketTrendToString(MarketTrend trend);
    static MarketTrend marketTrendFromString(const std::string& trendStr);

    static std::string sectorToString(Sector sector);
    static Sector sectorFromString(const std::string& sectorStr);
};

}