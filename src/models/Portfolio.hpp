#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "Company.hpp"
#include "Stock.hpp"
#include "Transaction.hpp"
#include "../utils/Date.hpp"

namespace StockMarketSimulator {

struct PortfolioPosition {
    std::shared_ptr<Company> company;
    int quantity;
    double averagePurchasePrice;
    double totalCost;
    double currentValue;
    double unrealizedProfitLoss;
    double unrealizedProfitLossPercent;
    Date purchaseDate;
    Date nextDividendDate;
    PortfolioPosition();

    PortfolioPosition(std::shared_ptr<Company> company, int quantity, double price, const Date &date);

    PortfolioPosition(std::shared_ptr<Company> company, int quantity, double price);

    void updatePosition(int addedQuantity, double price, const Date &date);
    void updateCurrentValue();
    void calculateUnrealizedProfitLoss();

    nlohmann::json toJson() const;
    static PortfolioPosition fromJson(const nlohmann::json& json, std::shared_ptr<Company> company);
};

struct PortfolioHistory {
    Date date;
    double totalValue;
    double cashBalance;
    double totalReturn;
    double totalReturnPercent;

    PortfolioHistory();
    PortfolioHistory(const Date& date, double totalValue, double cashBalance, double totalReturn, double totalReturnPercent);

    nlohmann::json toJson() const;
    static PortfolioHistory fromJson(const nlohmann::json& json);
};

class Portfolio {
private:
    std::unordered_map<std::string, PortfolioPosition> positions;
    std::vector<PortfolioHistory> history;
    std::vector<Transaction> transactions;

    double initialInvestment;
    double cashBalance;
    double totalValue;
    double previousDayValue;
    double totalDividendsReceived;

    void updatePortfolioValue();
    void recordHistoryEntry(const Date& date);

public:
    Portfolio();
    Portfolio(double initialBalance);

    double getInitialInvestment() const;
    double getCashBalance() const;
    double getTotalValue() const;
    double getTotalStocksValue() const;
    double getPreviousDayValue() const;
    double getTotalDividendsReceived() const;
    double getTotalReturn() const;
    double getTotalReturnPercent() const;
    double getDayChangeAmount() const;
    double getDayChangePercent() const;

    const std::unordered_map<std::string, PortfolioPosition>& getPositions() const;
    const std::vector<PortfolioHistory>& getHistory() const;
    const std::vector<Transaction>& getTransactions() const;

    bool hasPosition(const std::string& ticker) const;
    int getPositionQuantity(const std::string& ticker) const;
    const PortfolioPosition* getPosition(const std::string& ticker) const;
    PortfolioPosition* getPosition(std::string& ticker);
    bool buyStock(std::shared_ptr<Company> company, int quantity, double price, double commission, const Date& date);
    bool sellStock(std::shared_ptr<Company> company, int quantity, double price, double commission, const Date& date);

    void updatePositionValues();
    void closeDay(const Date& date);
    void openDay();

    void receiveDividends(std::shared_ptr<Company> company, double amount);

    void depositCash(double amount);
    bool withdrawCash(double amount);

    std::map<Sector, double> getSectorAllocation() const;
    double getSectorAllocationPercent(Sector sector) const;
    std::vector<double> getValueHistory() const;
    double getPeriodReturn(int days) const;
    double getPeriodReturnPercent(int days) const;

    nlohmann::json toJson() const;
    static Portfolio fromJson(const nlohmann::json& json, const std::vector<std::shared_ptr<Company>>& allCompanies);
    void increaseTotalDividendsReceived(double amount);
};

}