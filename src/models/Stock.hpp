#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include <ctime>
#include "../utils/Date.hpp"

namespace StockMarketSimulator {

class Company;

class Stock {
private:
    std::weak_ptr<Company> company;
    double currentPrice;
    std::vector<double> priceHistory;
    std::time_t lastUpdateTime;
    Date lastUpdateDate;

    double highestPrice;
    double lowestPrice;

    double previousClosePrice;
    double openPrice;

    double marketInfluence;
    double sectorInfluence;

    double dayChangeAmount;
    double dayChangePercent;

public:
    Stock();
    Stock(std::weak_ptr<Company> company, double initialPrice);
    Stock(const Stock& other);

    Stock& operator=(const Stock& other);

    double getCurrentPrice() const;
    double getPreviousClosePrice() const;
    double getOpenPrice() const;
    double getDayChangeAmount() const;
    double getDayChangePercent() const;
    double getHighestPrice() const;
    double getLowestPrice() const;
    std::vector<double> getPriceHistory() const;
    size_t getPriceHistoryLength() const;
    std::weak_ptr<Company> getCompany() const;
    Date getLastUpdateDate() const;

    void updatePrice(double newPrice);
    void calculateDailyChange();

    void updatePriceWithMarketInfluence(double marketTrend);
    void updatePriceWithSectorInfluence(double sectorTrend);
    void updatePriceWithNewsImpact(double newsImpact);
    void updatePriceWithVolatility(double volatility);

    void closeDay();
    void openDay();

    void closeDay(const Date& currentDate);
    void openDay(const Date& currentDate);

    void setMarketInfluence(double influence);
    void setSectorInfluence(double influence);

    double generatePriceMovement(double volatility, double marketTrend, double sectorTrend);

    nlohmann::json toJson() const;
    static Stock fromJson(const nlohmann::json& json, std::weak_ptr<Company> company);
};

}