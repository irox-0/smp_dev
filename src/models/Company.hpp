#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "Stock.hpp"
#include "../utils/Date.hpp"

namespace StockMarketSimulator {

enum class Sector {
    Technology,
    Energy,
    Finance,
    Consumer,
    Manufacturing,
    Unknown
};

struct DividendPolicy {
    double annualDividendRate;
    int paymentFrequency;
    int daysBetweenPayments;
    Date nextPaymentDay;

    DividendPolicy();
    DividendPolicy(double rate, int frequency);

    bool shouldPayDividend(const Date& currentDate) const;
    bool shouldPayDividend(int currentDay) const;

    double calculateDividendAmount() const;

    void scheduleNextPayment(const Date& currentDate);
    void scheduleNextPayment(int currentDay);

    nlohmann::json toJson() const;
    static DividendPolicy fromJson(const nlohmann::json& json);
};

struct DividendPaymentInfo {
        bool paymentMade;
        Date paymentDate;
        double amountPerShare;
    };

class Company : public std::enable_shared_from_this<Company> {
private:
    std::string name;
    std::string ticker;
    std::string description;
    Sector sector;
    double volatility;
    DividendPolicy dividendPolicy;
    std::unique_ptr<Stock> stock;
    Date lastDividendDate;

    double marketCap;
    double peRatio;
    double revenue;
    double profit;

    static Sector sectorFromString(const std::string& sectorStr);
    static std::string sectorToString(Sector sector);

public:
    Company();
    Company(const std::string& name, const std::string& ticker,
            const std::string& description, Sector sector,
            double initialPrice, double volatility,
            const DividendPolicy& dividendPolicy);
    Company(const Company& other);

    Company& operator=(const Company& other);

    std::string getName() const;
    std::string getTicker() const;
    std::string getDescription() const;
    Sector getSector() const;
    std::string getSectorName() const;
    double getVolatility() const;
    const DividendPolicy& getDividendPolicy() const;
    Stock* getStock() const;
    double getMarketCap() const;
    double getPERatio() const;
    double getRevenue() const;
    double getProfit() const;

    void setName(const std::string& name);
    void setTicker(const std::string& ticker);
    void setDescription(const std::string& description);
    void setSector(Sector sector);
    void setVolatility(double volatility);
    void setDividendPolicy(const DividendPolicy& policy);
    void setFinancials(double marketCap, double peRatio, double revenue, double profit);

    void updateStockPrice(double marketTrend, double sectorTrend);
    void processNewsImpact(double newsImpact);

    void closeTradingDay();
    void openTradingDay();

    void closeTradingDay(const Date& currentDate);
    void openTradingDay(const Date& currentDate);

    bool processDividends(const Date& currentDate);
    bool processDividends(int currentDay);

    double calculateDividendAmount() const;
    void initializeDividendSchedule(const Date& currentDate);
    nlohmann::json toJson() const;
    static std::shared_ptr<Company> fromJson(const nlohmann::json& json);
    Date getLastDividendDate() const { return lastDividendDate; }
};

}