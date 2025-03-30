#include "Market.hpp"
#include "../utils/Random.hpp"
#include "../utils/FileIO.hpp"
#include <algorithm>
#include <cmath>

namespace StockMarketSimulator {

Market::Market()
    : currentDate(1, 3, 2023),
      cycleLength(365),
      currentCycleDay(0),
      marketVolatility(0.01),
      marketMomentum(0.3),
      trendStrength(0.7)
{
    state.indexValue = 1000.0;
    state.dailyChange = 0.0;
    state.dailyChangePercent = 0.0;
    state.currentTrend = MarketTrend::Sideways;
    state.trendDuration = 0;
    state.interestRate = 0.05;
    state.inflationRate = 0.02;
    state.unemploymentRate = 0.045;

    sectorTrends[Sector::Technology] = 0.0;
    sectorTrends[Sector::Energy] = 0.0;
    sectorTrends[Sector::Finance] = 0.0;
    sectorTrends[Sector::Consumer] = 0.0;
    sectorTrends[Sector::Manufacturing] = 0.0;
}

const MarketState& Market::getState() const {
    return state;
}

const std::vector<std::shared_ptr<Company>>& Market::getCompanies() const {
    return companies;
}

const std::map<Sector, double>& Market::getSectorTrends() const {
    return sectorTrends;
}

Date Market::getCurrentDate() const {
    return currentDate;
}

int Market::getCurrentDay() const {
    Date referenceDate(1, 3, 2023);
    return currentDate.toDayNumber(referenceDate) - 1;
}

double Market::getMarketIndex() const {
    return state.indexValue;
}

MarketTrend Market::getCurrentTrend() const {
    return state.currentTrend;
}

std::string Market::getTrendName() const {
    return marketTrendToString(state.currentTrend);
}

double Market::getInterestRate() const {
    return state.interestRate;
}

double Market::getInflationRate() const {
    return state.inflationRate;
}

double Market::getUnemploymentRate() const {
    return state.unemploymentRate;
}

void Market::addCompany(std::shared_ptr<Company> company) {
    companies.push_back(company);
}

void Market::removeCompany(const std::string& ticker) {
    companies.erase(
        std::remove_if(companies.begin(), companies.end(),
            [&](const std::shared_ptr<Company>& company) {
                return company->getTicker() == ticker;
            }),
        companies.end()
    );
}

std::shared_ptr<Company> Market::getCompanyByTicker(const std::string& ticker) const {
    auto it = std::find_if(companies.begin(), companies.end(),
        [&](const std::shared_ptr<Company>& company) {
            return company->getTicker() == ticker;
        });

    return (it != companies.end()) ? *it : nullptr;
}

std::vector<std::shared_ptr<Company>> Market::getCompaniesBySector(Sector sector) const {
    std::vector<std::shared_ptr<Company>> sectorCompanies;

    for (const auto& company : companies) {
        if (company->getSector() == sector) {
            sectorCompanies.push_back(company);
        }
    }

    return sectorCompanies;
}

void Market::addDefaultCompanies() {
    auto techCorp = std::make_shared<Company>(
        "TechCorp", "TCH",
        "A leading technology company specializing in software and cloud services",
        Sector::Technology,
        85.75, 0.75,
        DividendPolicy(1.6, 4)
    );

    auto innovaTech = std::make_shared<Company>(
        "InnovaTech", "ITECH",
        "An innovative company developing mobile devices and applications",
        Sector::Technology,
        125.30, 0.85,
        DividendPolicy(0.0, 0)
    );

    auto energyPlus = std::make_shared<Company>(
        "EnergyPlus", "EPLC",
        "An energy company specializing in renewable energy sources",
        Sector::Energy,
        45.20, 0.5,
        DividendPolicy(3.5, 4)
    );

    auto oilMax = std::make_shared<Company>(
        "OilMax", "OMAX",
        "An oil and gas company with a global presence",
        Sector::Energy,
        76.45, 0.6,
        DividendPolicy(4.2, 4)
    );

    auto bankCo = std::make_shared<Company>(
        "BankCo", "BANK",
        "A large commercial bank with a wide range of financial services",
        Sector::Finance,
        32.15, 0.45,
        DividendPolicy(3.8, 4)
    );

    auto secureFin = std::make_shared<Company>(
        "SecureFin", "SFIN",
        "A financial company specializing in insurance and investments",
        Sector::Finance,
        54.80, 0.5,
        DividendPolicy(2.5, 4)
    );

    auto retailGiant = std::make_shared<Company>(
        "RetailGiant", "RTGL",
        "The largest chain of retail stores",
        Sector::Consumer,
        23.50, 0.4,
        DividendPolicy(2.2, 4)
    );

    auto foodCorp = std::make_shared<Company>(
        "FoodCorp", "FOOD",
        "Food Manufacturer and Distributor",
        Sector::Consumer,
        34.60, 0.35,
        DividendPolicy(1.8, 4)
    );

    auto industrialCo = std::make_shared<Company>(
        "IndustrialCo", "INDL",
        "An industrial company that manufactures equipment and machinery",
        Sector::Manufacturing,
        67.90, 0.55,
        DividendPolicy(2.8, 4)
    );

    addCompany(techCorp);
    addCompany(innovaTech);
    addCompany(energyPlus);
    addCompany(oilMax);
    addCompany(bankCo);
    addCompany(secureFin);
    addCompany(retailGiant);
    addCompany(foodCorp);
    addCompany(industrialCo);
}

void Market::simulateDay() {
    for (auto& company : companies) {
        company->closeTradingDay(currentDate);
    }

    currentDate.nextDay();
    currentCycleDay = (currentCycleDay + 1) % cycleLength;

    calculateMarketTrend();

    updateMacroeconomicFactors();

    double marketMovement = generateMarketMovement();

    double previousIndex = state.indexValue;
    state.indexValue *= (1.0 + marketMovement);
    state.dailyChange = state.indexValue - previousIndex;
    state.dailyChangePercent = (state.dailyChange / previousIndex) * 100.0;

    updateSectorTrends();

    for (auto& company : companies) {
        company->openTradingDay(currentDate);

        Sector companySector = company->getSector();
        double sectorTrend = sectorTrends[companySector];
        company->updateStockPrice(marketMovement, sectorTrend);
    }
}

void Market::processCompanyDividends() {
    for (auto& company : companies) {
        if (company->processDividends(currentDate)) {
        }
    }
}

void Market::setMarketTrend(MarketTrend trend) {
    state.currentTrend = trend;
    state.trendDuration = 0;
}

void Market::triggerEconomicEvent(double impact, bool affectAllSectors) {
    state.indexValue *= (1.0 + impact);

    if (affectAllSectors) {
        for (auto& sector : sectorTrends) {
            sector.second += impact;
        }
    } else {
        std::vector<Sector> sectors = {
            Sector::Technology, Sector::Energy, Sector::Finance,
            Sector::Consumer, Sector::Manufacturing
        };

        Sector affectedSector = Random::getRandomElement(sectors);
        sectorTrends[affectedSector] += impact * 2.0;
    }

    for (auto& company : companies) {
        Sector companySector = company->getSector();
        company->updateStockPrice(impact, sectorTrends[companySector]);
    }
}

void Market::updateMarketIndex() {
    if (companies.empty()) {
        return;
    }

    double totalMarketCap = 0.0;
    for (const auto& company : companies) {
        totalMarketCap += company->getMarketCap();
    }

    state.indexValue = (totalMarketCap / companies.size()) / 10;
}

void Market::updateSectorTrends() {
    for (auto& [sector, trend] : sectorTrends) {
        double newTrend = generateSectorMovement(sector);

        if (newTrend == trend) {
            newTrend += Random::getDouble(-0.01, 0.01);
        }

        trend = newTrend;
    }
}

void Market::calculateMarketTrend() {
    state.trendDuration++;

    double changeProbability = std::min(0.05 + (state.trendDuration / 100.0), 0.3);

    if (Random::getBool(changeProbability)) {
        double rand = Random::getDouble(0.0, 1.0);

        if (rand < 0.35) {
            state.currentTrend = MarketTrend::Bullish;
        } else if (rand < 0.7) {
            state.currentTrend = MarketTrend::Bearish;
        } else if (rand < 0.85) {
            state.currentTrend = MarketTrend::Sideways;
        } else {
            state.currentTrend = MarketTrend::Volatile;
        }

        state.trendDuration = 0;
    }
}

void Market::updateMacroeconomicFactors() {
    double interestRateChange = Random::getNormal(0.0, 0.001);
    state.interestRate = std::max(0.01, std::min(0.15, state.interestRate + interestRateChange));

    double inflationChange = Random::getNormal(0.0, 0.001);
    state.inflationRate = std::max(0.0, std::min(0.2, state.inflationRate + inflationChange));

    double unemploymentChange = Random::getNormal(0.0, 0.001);
    state.unemploymentRate = std::max(0.02, std::min(0.15, state.unemploymentRate + unemploymentChange));

    if (state.inflationRate > 0.05 && state.interestRate < 0.1) {
        state.interestRate += Random::getDouble(0.001, 0.003);
    }

    if (state.interestRate > 0.08 && state.unemploymentRate < 0.08) {
        state.unemploymentRate += Random::getDouble(0.001, 0.002);
    }

    if (state.unemploymentRate > 0.08 && state.inflationRate > 0.02) {
        state.inflationRate -= Random::getDouble(0.001, 0.002);
    }
}

double Market::generateMarketMovement() {
    double baseMovement = 0.0;

    switch (state.currentTrend) {
        case MarketTrend::Bullish:
            baseMovement = Random::getNormal(0.002, marketVolatility);
            break;
        case MarketTrend::Bearish:
            baseMovement = Random::getNormal(-0.002, marketVolatility);
            break;
        case MarketTrend::Sideways:
            baseMovement = Random::getNormal(0.0, marketVolatility * 0.5);
            break;
        case MarketTrend::Volatile:
            baseMovement = Random::getNormal(0.0, marketVolatility * 2.0);
            break;
        default:
            baseMovement = Random::getNormal(0.0, marketVolatility);
    }

    double cyclicalComponent = std::sin(2.0 * M_PI * currentCycleDay / cycleLength) * 0.001;

    double macroComponent = 0.0;

    if (state.inflationRate > 0.05) {
        macroComponent -= (state.inflationRate - 0.05) * 0.1;
    }

    if (state.unemploymentRate > 0.06) {
        macroComponent -= (state.unemploymentRate - 0.06) * 0.1;
    }

    macroComponent -= (state.interestRate - 0.05) * 0.1;

    double totalMovement = (baseMovement + cyclicalComponent + macroComponent);

    return totalMovement;
}

double Market::generateSectorMovement(Sector sector) {
    double marketMovement = (state.dailyChangePercent / 100.0);

    double randomComponent = Random::getNormal(0.0, 0.005);

    double sectorSpecific = 0.0;

    switch (sector) {
        case Sector::Technology:
            sectorSpecific = randomComponent * 1.5;
            sectorSpecific -= (state.interestRate - 0.05) * 0.2;
            break;

        case Sector::Energy:
            sectorSpecific = std::sin(2.0 * M_PI * currentCycleDay / cycleLength) * 0.002;
            break;

        case Sector::Finance:
            sectorSpecific += (state.interestRate - 0.05) * 0.1;
            sectorSpecific -= (state.unemploymentRate - 0.05) * 0.2;
            break;

        case Sector::Consumer:
            sectorSpecific -= (state.unemploymentRate - 0.05) * 0.3;
            sectorSpecific -= (state.inflationRate - 0.02) * 0.2;
            break;

        case Sector::Manufacturing:
            sectorSpecific = std::sin(2.0 * M_PI * currentCycleDay / cycleLength) * 0.001;
            sectorSpecific -= (state.inflationRate - 0.02) * 0.1;
            break;

        default:
            break;
    }

    double totalMovement = (marketMovement * 0.6) + (sectorSpecific * 0.4);

    return totalMovement;
}

nlohmann::json Market::toJson() const {
    nlohmann::json j;

    j["current_date"] = currentDate.toJson();
    j["cycle_length"] = cycleLength;
    j["current_cycle_day"] = currentCycleDay;

    j["market_state"] = {
        {"index_value", state.indexValue},
        {"daily_change", state.dailyChange},
        {"daily_change_percent", state.dailyChangePercent},
        {"current_trend", marketTrendToString(state.currentTrend)},
        {"trend_duration", state.trendDuration},
        {"interest_rate", state.interestRate},
        {"inflation_rate", state.inflationRate},
        {"unemployment_rate", state.unemploymentRate}
    };

    j["sector_trends"] = nlohmann::json::object();
    for (const auto& [sector, trend] : sectorTrends) {
        j["sector_trends"][sectorToString(sector)] = trend;
    }

    j["companies"] = nlohmann::json::array();
    for (const auto& company : companies) {
        j["companies"].push_back(company->toJson());
    }

    return j;
}

Market Market::fromJson(const nlohmann::json& json) {
    Market market;

    if (json.contains("current_date")) {
        market.currentDate = Date::fromJson(json["current_date"]);
    } else if (json.contains("current_day")) {
        int currentDay = json["current_day"];
        market.currentDate = Date::fromDayNumber(currentDay + 1, Date(1, 3, 2023));
    } else {
        market.currentDate = Date(1, 3, 2023);
    }

    market.cycleLength = json["cycle_length"];
    market.currentCycleDay = json["current_cycle_day"];

    auto& stateJson = json["market_state"];
    market.state.indexValue = stateJson["index_value"];
    market.state.dailyChange = stateJson["daily_change"];
    market.state.dailyChangePercent = stateJson["daily_change_percent"];
    market.state.currentTrend = marketTrendFromString(stateJson["current_trend"]);
    market.state.trendDuration = stateJson["trend_duration"];
    market.state.interestRate = stateJson["interest_rate"];
    market.state.inflationRate = stateJson["inflation_rate"];
    market.state.unemploymentRate = stateJson["unemployment_rate"];

    for (const auto& [sectorStr, trend] : json["sector_trends"].items()) {
        Sector sector = sectorFromString(sectorStr);
        market.sectorTrends[sector] = trend;
    }

    market.companies.clear();
    for (const auto& companyJson : json["companies"]) {
        market.companies.push_back(Company::fromJson(companyJson));
    }

    return market;
}

std::string Market::marketTrendToString(MarketTrend trend) {
    switch (trend) {
        case MarketTrend::Bullish: return "Bullish";
        case MarketTrend::Bearish: return "Bearish";
        case MarketTrend::Sideways: return "Sideways";
        case MarketTrend::Volatile: return "Volatile";
        default: return "Unknown";
    }
}

MarketTrend Market::marketTrendFromString(const std::string& trendStr) {
    if (trendStr == "Bullish") return MarketTrend::Bullish;
    if (trendStr == "Bearish") return MarketTrend::Bearish;
    if (trendStr == "Sideways") return MarketTrend::Sideways;
    if (trendStr == "Volatile") return MarketTrend::Volatile;

    return MarketTrend::Sideways;
}

std::string Market::sectorToString(Sector sector) {
    switch (sector) {
        case Sector::Technology: return "Technology";
        case Sector::Energy: return "Energy";
        case Sector::Finance: return "Finance";
        case Sector::Consumer: return "Consumer";
        case Sector::Manufacturing: return "Manufacturing";
        default: return "Unknown";
    }
}

Sector Market::sectorFromString(const std::string& sectorStr) {
    if (sectorStr == "Technology") return Sector::Technology;
    if (sectorStr == "Energy") return Sector::Energy;
    if (sectorStr == "Finance") return Sector::Finance;
    if (sectorStr == "Consumer") return Sector::Consumer;
    if (sectorStr == "Manufacturing") return Sector::Manufacturing;

    return Sector::Unknown;
}

}