#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <nlohmann/json.hpp>
#include "../models/Company.hpp"

namespace StockMarketSimulator {

enum class MarketTrend {
    Bullish,      // Растущий рынок
    Bearish,      // Падающий рынок
    Sideways,     // Боковой тренд
    Volatile      // Волатильный рынок
};

struct MarketState {
    double indexValue;              // Текущее значение индекса рынка
    double dailyChange;             // Изменение индекса за день
    double dailyChangePercent;      // Процентное изменение индекса за день
    MarketTrend currentTrend;       // Текущий тренд рынка
    int trendDuration;              // Длительность текущего тренда в днях
    double interestRate;            // Процентная ставка
    double inflationRate;           // Уровень инфляции
    double unemploymentRate;        // Уровень безработицы
};

class Market {
private:
    std::vector<std::shared_ptr<Company>> companies;
    MarketState state;
    std::map<Sector, double> sectorTrends;
    int currentDay;
    int cycleLength;               // Длина экономического цикла в днях
    int currentCycleDay;           // Текущий день в цикле
    
    // Параметры для генерации движения рынка
    double marketVolatility;        // Общая волатильность рынка
    double marketMomentum;          // Инерция рынка (влияние предыдущего движения)
    double trendStrength;           // Сила текущего тренда
    
    // Приватные методы
    void updateMarketIndex();
    void updateSectorTrends();
    void calculateMarketTrend();
    void updateMacroeconomicFactors();
    double generateMarketMovement();
    double generateSectorMovement(Sector sector);
    
public:
    Market();
    
    // Геттеры
    const MarketState& getState() const;
    const std::vector<std::shared_ptr<Company>>& getCompanies() const;
    const std::map<Sector, double>& getSectorTrends() const;
    int getCurrentDay() const;
    double getMarketIndex() const;
    MarketTrend getCurrentTrend() const;
    std::string getTrendName() const;
    double getInterestRate() const;
    double getInflationRate() const;
    double getUnemploymentRate() const;
    
    // Методы для работы с компаниями
    void addCompany(std::shared_ptr<Company> company);
    void removeCompany(const std::string& ticker);
    std::shared_ptr<Company> getCompanyByTicker(const std::string& ticker) const;
    std::vector<std::shared_ptr<Company>> getCompaniesBySector(Sector sector) const;
    void addDefaultCompanies();
    
    // Методы для симуляции рынка
    void simulateDay();
    void processCompanyDividends();
    void setMarketTrend(MarketTrend trend);
    void triggerEconomicEvent(double impact, bool affectAllSectors = true);
    
    // Методы для сохранения/загрузки
    nlohmann::json toJson() const;
    static Market fromJson(const nlohmann::json& json);
    
    // Статические вспомогательные методы
    static std::string marketTrendToString(MarketTrend trend);
    static MarketTrend marketTrendFromString(const std::string& trendStr);

    // Вспомогательные методы для работы с секторами (так как методы в Company приватные)
    static std::string sectorToString(Sector sector);
    static Sector sectorFromString(const std::string& sectorStr);
};

}