#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "Stock.hpp"
#include "Company.hpp"
#include "../utils/Date.hpp"

namespace StockMarketSimulator {

enum class TransactionType {
    Buy,
    Sell
};

class Transaction {
private:
    TransactionType type;
    std::weak_ptr<Company> company;
    int quantity;
    double pricePerShare;
    double commissionRate;
    double commissionAmount;
    double totalCost;
    Date transactionDate;
    bool executed;
    std::string status;

    void calculateCommission();
    void calculateTotalCost();

public:
    Transaction();
    Transaction(TransactionType type, std::weak_ptr<Company> company,
                int quantity, double pricePerShare, double commissionRate,
                const Date& transactionDate);

    TransactionType getType() const;
    std::weak_ptr<Company> getCompany() const;
    int getQuantity() const;
    double getPricePerShare() const;
    double getCommissionRate() const;
    double getCommissionAmount() const;
    double getTotalCost() const;
    Date getTransactionDate() const;
    bool isExecuted() const;
    std::string getStatus() const;

    void setType(TransactionType type);
    void setCompany(std::weak_ptr<Company> company);
    void setQuantity(int quantity);
    void setPricePerShare(double price);
    void setCommissionRate(double rate);
    void setTransactionDate(const Date& date);
    void setExecuted(bool executed);
    void setStatus(const std::string& status);

    bool validateTransaction(double availableFunds);
    bool validateSellTransaction(int availableShares);
    void execute();

    static double calculateTotalWithCommission(double price, int quantity, double commissionRate);
    static std::string transactionTypeToString(TransactionType type);
    static TransactionType transactionTypeFromString(const std::string& typeStr);

    nlohmann::json toJson() const;
    static Transaction fromJson(const nlohmann::json& json);
};

}