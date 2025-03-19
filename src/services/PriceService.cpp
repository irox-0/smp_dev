#include "PriceService.hpp"
#include <cmath>
#include <algorithm>

namespace StockMarketSimulator {

nlohmann::json SectorVolatilityProfile::toJson() const {
    nlohmann::json j;
    j["base_volatility"] = baseVolatility;
    j["market_sensitivity"] = marketSensitivity;
    j["news_sensitivity"] = newsSensitivity;
    j["cycle_sensitivity"] = cycleSensitivity;
    return j;
}

SectorVolatilityProfile SectorVolatilityProfile::fromJson(const nlohmann::json& json) {
    SectorVolatilityProfile profile;
    profile.baseVolatility = json["base_volatility"];
    profile.marketSensitivity = json["market_sensitivity"];
    profile.newsSensitivity = json["news_sensitivity"];
    profile.cycleSensitivity = json["cycle_sensitivity"];
    return profile;
}

nlohmann::json EconomicCycleParams::toJson() const {
    nlohmann::json j;
    j["cycle_length"] = cycleLength;
    j["current_position"] = currentPosition;
    j["amplitude"] = amplitude;
    j["phase_shift"] = phaseShift;
    return j;
}

EconomicCycleParams EconomicCycleParams::fromJson(const nlohmann::json& json) {
    EconomicCycleParams params;
    params.cycleLength = json["cycle_length"];
    params.currentPosition = json["current_position"];
    params.amplitude = json["amplitude"];
    params.phaseShift = json["phase_shift"];
    return params;
}

PriceService::PriceService()
    : marketVolatilityFactor(0.01),
      trendStrength(0.6),
      momentumFactor(0.3),
      randomnessFactor(0.5),
      economicCycle(365, 0, 0.02, 0.0)
{
    initializeSectorProfiles();
}

PriceService::PriceService(std::weak_ptr<Market> market)
    : market(market),
      marketVolatilityFactor(0.01),
      trendStrength(0.6),
      momentumFactor(0.3),
      randomnessFactor(0.5),
      economicCycle(365, 0, 0.02, 0.0)
{
    initializeSectorProfiles();
}

void PriceService::initialize() {
    initializeSectorProfiles();
}

void PriceService::setMarket(std::weak_ptr<Market> market) {
    this->market = market;
}

void PriceService::initializeSectorProfiles() {
    sectorProfiles[Sector::Technology] = SectorVolatilityProfile(0.05, 0.8, 0.9, 0.3);

    sectorProfiles[Sector::Energy] = SectorVolatilityProfile(0.015, 0.5, 0.6, 0.8);

    sectorProfiles[Sector::Finance] = SectorVolatilityProfile(0.008, 0.8, 0.7, 0.4);

    sectorProfiles[Sector::Consumer] = SectorVolatilityProfile(0.008, 0.4, 0.7, 0.3);

    sectorProfiles[Sector::Manufacturing] = SectorVolatilityProfile(0.012, 0.6, 0.5, 0.6);

    sectorProfiles[Sector::Unknown] = SectorVolatilityProfile(0.01, 0.5, 0.5, 0.5);
}

void PriceService::updatePrices() {
    auto marketPtr = market.lock();
    if (!marketPtr) {
        return;
    }

    MarketTrend currentTrend = marketPtr->getCurrentTrend();
    const auto& companies = marketPtr->getCompanies();

    for (const auto& company : companies) {
        double priceMovement = generatePriceMovement(company, currentTrend);

        Stock* stock = company->getStock();
        double currentPrice = stock->getCurrentPrice();

        double newPrice = currentPrice * (1.0 + priceMovement);

        stock->updatePrice(newPrice);
    }

    advanceEconomicCycle();
}

double PriceService::generatePriceMovement(const std::shared_ptr<Company>& company, MarketTrend trend) {
    if (!company) {
        return 0.0;
    }

    Sector sector = company->getSector();

    const auto& profile = sectorProfiles[sector];

    double trendComponent = generateTrendComponent(trend) * trendStrength * profile.marketSensitivity;
    double cyclicalComponent = generateCyclicalComponent() * profile.cycleSensitivity;
    double randomComponent = generateRandomComponent(profile.baseVolatility) * randomnessFactor;
    double sectorComponent = generateSectorComponent(sector, trend);

    double totalMovement = trendComponent + cyclicalComponent + randomComponent + sectorComponent;

    totalMovement = calculateMomentumEffect(company->getTicker(), totalMovement);

    if (trend == MarketTrend::Bullish) {
        totalMovement = std::max(totalMovement, 0.001);
    }

    double maxChange = 0.1;
    totalMovement = std::max(std::min(totalMovement, maxChange), -maxChange);

    priceMovementHistory[company->getTicker()].push_back(totalMovement);

    const size_t MAX_HISTORY_SIZE = 100;
    if (priceMovementHistory[company->getTicker()].size() > MAX_HISTORY_SIZE) {
        priceMovementHistory[company->getTicker()].erase(
            priceMovementHistory[company->getTicker()].begin());
    }

    return totalMovement;
}

double PriceService::generateTrendComponent(MarketTrend trend) const {
    switch (trend) {
        case MarketTrend::Bullish:
            return Random::getNormal(0.005, marketVolatilityFactor);
        case MarketTrend::Bearish:
            return Random::getNormal(-0.004, marketVolatilityFactor);
        case MarketTrend::Sideways:
            return Random::getNormal(0.0, marketVolatilityFactor * 0.5);
        case MarketTrend::Volatile:
            return Random::getNormal(0.0, marketVolatilityFactor * 2.0);
        default:
            return Random::getNormal(0.0, marketVolatilityFactor);
    }
}

double PriceService::generateCyclicalComponent() const {
    double phase = (static_cast<double>(economicCycle.currentPosition) / economicCycle.cycleLength) * 2.0 * M_PI;
    phase += economicCycle.phaseShift;

    return std::sin(phase) * economicCycle.amplitude;
}

double PriceService::generateRandomComponent(double volatility) const {
    return Random::getNormal(0.0, volatility);
}

double PriceService::generateSectorComponent(Sector sector, MarketTrend trend) const {
    auto marketPtr = market.lock();
    if (!marketPtr) {
        return 0.0;
    }

    const auto& sectorTrends = marketPtr->getSectorTrends();
    auto it = sectorTrends.find(sector);

    if (it != sectorTrends.end()) {
        double sectorTrend = it->second;

        if (trend == MarketTrend::Bullish && sectorTrend > 0) {
            sectorTrend *= 1.2;
        } else if (trend == MarketTrend::Bearish && sectorTrend < 0) {
            sectorTrend *= 1.2;
        }

        if (sector == Sector::Technology) {
            sectorTrend += Random::getNormal(0.002, 0.01);
        }
        
        return sectorTrend * 0.5;
    }
    
    return 0.0;
}

double PriceService::calculateMomentumEffect(const std::string& ticker, double newMovement) {
    const auto& history = priceMovementHistory[ticker];
    
    if (history.empty()) {
        return newMovement;
    }
    
    int lookback = std::min(5, static_cast<int>(history.size()));
    double recentAverage = 0.0;
    
    for (int i = static_cast<int>(history.size()) - 1; i >= static_cast<int>(history.size()) - lookback; --i) {
        recentAverage += history[i];
    }
    
    recentAverage /= lookback;
    
    return newMovement * (1.0 - momentumFactor) + recentAverage * momentumFactor;
}

void PriceService::advanceEconomicCycle(int days) {
    economicCycle.currentPosition = (economicCycle.currentPosition + days) % economicCycle.cycleLength;
}

void PriceService::simulateMarketShock(double magnitude, bool positive) {
    auto marketPtr = market.lock();
    if (!marketPtr) {
        return;
    }
    
    double impact = positive ? std::abs(magnitude) : -std::abs(magnitude);
    
    marketPtr->triggerEconomicEvent(impact);
}

void PriceService::simulateSectorShock(Sector sector, double magnitude, bool positive) {
    auto marketPtr = market.lock();
    if (!marketPtr) {
        return;
    }

    double impact = positive ? std::abs(magnitude) : -std::abs(magnitude);

    const auto& sectorCompanies = marketPtr->getCompaniesBySector(sector);

    for (const auto& company : sectorCompanies) {
        if (sector == Sector::Technology && positive) {
            for (int i = 0; i < 3; i++) {
                company->processNewsImpact(impact);
            }
        } else {
            company->processNewsImpact(impact);
        }
    }
}
double PriceService::getMarketVolatilityFactor() const {
    return marketVolatilityFactor;
}

void PriceService::setMarketVolatilityFactor(double factor) {
    if (factor > 0.0) {
        marketVolatilityFactor = factor;
    }
}

double PriceService::getTrendStrength() const {
    return trendStrength;
}

void PriceService::setTrendStrength(double strength) {
    if (strength >= 0.0 && strength <= 1.0) {
        trendStrength = strength;
    }
}

double PriceService::getMomentumFactor() const {
    return momentumFactor;
}

void PriceService::setMomentumFactor(double factor) {
    if (factor >= 0.0 && factor <= 1.0) {
        momentumFactor = factor;
    }
}

double PriceService::getRandomnessFactor() const {
    return randomnessFactor;
}

void PriceService::setRandomnessFactor(double factor) {
    if (factor >= 0.0 && factor <= 1.0) {
        randomnessFactor = factor;
    }
}

const EconomicCycleParams& PriceService::getEconomicCycleParams() const {
    return economicCycle;
}

void PriceService::setEconomicCycleParams(const EconomicCycleParams& params) {
    economicCycle = params;
}

const SectorVolatilityProfile& PriceService::getSectorProfile(Sector sector) const {
    auto it = sectorProfiles.find(sector);
    if (it != sectorProfiles.end()) {
        return it->second;
    }
    
    return sectorProfiles.at(Sector::Unknown);
}

void PriceService::setSectorProfile(Sector sector, const SectorVolatilityProfile& profile) {
    sectorProfiles[sector] = profile;
}

const std::vector<double>& PriceService::getPriceMovementHistory(const std::string& ticker) const {
    auto it = priceMovementHistory.find(ticker);
    if (it != priceMovementHistory.end()) {
        return it->second;
    }
    
    static const std::vector<double> emptyHistory;
    return emptyHistory;
}

nlohmann::json PriceService::toJson() const {
    nlohmann::json j;
    
    j["market_volatility_factor"] = marketVolatilityFactor;
    j["trend_strength"] = trendStrength;
    j["momentum_factor"] = momentumFactor;
    j["randomness_factor"] = randomnessFactor;
    j["economic_cycle"] = economicCycle.toJson();
    
    j["sector_profiles"] = nlohmann::json::object();
    for (const auto& [sector, profile] : sectorProfiles) {
        j["sector_profiles"][Market::sectorToString(sector)] = profile.toJson();
    }
    
    j["price_movement_history"] = nlohmann::json::object();
    for (const auto& [ticker, history] : priceMovementHistory) {
        j["price_movement_history"][ticker] = history;
    }
    
    return j;
}

PriceService PriceService::fromJson(const nlohmann::json& json, std::weak_ptr<Market> market) {
    PriceService service(market);
    
    service.marketVolatilityFactor = json["market_volatility_factor"];
    service.trendStrength = json["trend_strength"];
    service.momentumFactor = json["momentum_factor"];
    service.randomnessFactor = json["randomness_factor"];
    service.economicCycle = EconomicCycleParams::fromJson(json["economic_cycle"]);
    
    for (auto it = json["sector_profiles"].begin(); it != json["sector_profiles"].end(); ++it) {
        Sector sector = Market::sectorFromString(it.key());
        service.sectorProfiles[sector] = SectorVolatilityProfile::fromJson(it.value());
    }
    
    for (auto it = json["price_movement_history"].begin(); it != json["price_movement_history"].end(); ++it) {
        std::string ticker = it.key();
        service.priceMovementHistory[ticker] = it.value().get<std::vector<double>>();
    }
    
    return service;
}

}