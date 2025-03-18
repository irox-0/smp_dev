#include "Transaction.hpp"
#include <stdexcept>
#include <cmath>

namespace StockMarketSimulator {

Transaction::Transaction()
    : type(TransactionType::Buy),
      quantity(0),
      pricePerShare(0.0),
      commissionRate(0.01),
      commissionAmount(0.0),
      totalCost(0.0),
      transactionDay(0),
      executed(false),
      status("Initialized")
{
}

Transaction::Transaction(TransactionType type, std::weak_ptr<Company> company, 
                         int quantity, double pricePerShare, double commissionRate,
                         int transactionDay)
    : type(type),
      company(company),
      quantity(std::max(0, quantity)),
      pricePerShare(std::max(0.0, pricePerShare)),
      commissionRate(std::max(0.0, std::min(0.1, commissionRate))),
      transactionDay(std::max(0, transactionDay)),
      executed(false),
      status("Initialized")
{
    calculateCommission();
    calculateTotalCost();
}

void Transaction::calculateCommission() {
    commissionAmount = pricePerShare * quantity * commissionRate;
}

void Transaction::calculateTotalCost() {
    double baseCost = pricePerShare * quantity;
    
    if (type == TransactionType::Buy) {
        totalCost = baseCost + commissionAmount;
    } else {
        totalCost = baseCost - commissionAmount;
    }
}

TransactionType Transaction::getType() const {
    return type;
}

std::weak_ptr<Company> Transaction::getCompany() const {
    return company;
}

int Transaction::getQuantity() const {
    return quantity;
}

double Transaction::getPricePerShare() const {
    return pricePerShare;
}

double Transaction::getCommissionRate() const {
    return commissionRate;
}

double Transaction::getCommissionAmount() const {
    return commissionAmount;
}

double Transaction::getTotalCost() const {
    return totalCost;
}

int Transaction::getTransactionDay() const {
    return transactionDay;
}

bool Transaction::isExecuted() const {
    return executed;
}

std::string Transaction::getStatus() const {
    return status;
}

void Transaction::setType(TransactionType type) {
    this->type = type;
    calculateTotalCost();
}

void Transaction::setCompany(std::weak_ptr<Company> company) {
    this->company = company;
}

void Transaction::setQuantity(int quantity) {
    if (quantity >= 0) {
        this->quantity = quantity;
        calculateCommission();
        calculateTotalCost();
    }
}

void Transaction::setPricePerShare(double price) {
    if (price >= 0.0) {
        this->pricePerShare = price;
        calculateCommission();
        calculateTotalCost();
    }
}

void Transaction::setCommissionRate(double rate) {
    if (rate >= 0.0 && rate <= 0.1) {
        this->commissionRate = rate;
        calculateCommission();
        calculateTotalCost();
    }
}

void Transaction::setTransactionDay(int day) {
    if (day >= 0) {
        this->transactionDay = day;
    }
}

void Transaction::setExecuted(bool executed) {
    this->executed = executed;
}

void Transaction::setStatus(const std::string& status) {
    this->status = status;
}

bool Transaction::validateTransaction(double availableFunds) {
    if (executed) {
        status = "Transaction already executed";
        return false;
    }
    
    if (quantity <= 0) {
        status = "Invalid quantity";
        return false;
    }
    
    auto companyPtr = company.lock();
    if (!companyPtr) {
        status = "Invalid company reference";
        return false;
    }
    
    if (type == TransactionType::Buy) {
        if (totalCost > availableFunds) {
            status = "Insufficient funds";
            return false;
        }
    }
    
    status = "Valid";
    return true;
}

bool Transaction::validateSellTransaction(int availableShares) {
    if (type != TransactionType::Sell) {
        status = "Not a sell transaction";
        return false;
    }
    
    if (executed) {
        status = "Transaction already executed";
        return false;
    }
    
    if (quantity <= 0) {
        status = "Invalid quantity";
        return false;
    }
    
    auto companyPtr = company.lock();
    if (!companyPtr) {
        status = "Invalid company reference";
        return false;
    }
    
    if (quantity > availableShares) {
        status = "Insufficient shares";
        return false;
    }
    
    status = "Valid";
    return true;
}

void Transaction::execute() {
    if (!executed) {
        executed = true;
        status = "Executed";
    } else {
        throw std::runtime_error("Transaction already executed");
    }
}

double Transaction::calculateTotalWithCommission(double price, int quantity, double commissionRate) {
    double baseCost = price * quantity;
    double commission = baseCost * commissionRate;
    return baseCost + commission;
}

std::string Transaction::transactionTypeToString(TransactionType type) {
    switch (type) {
        case TransactionType::Buy: return "Buy";
        case TransactionType::Sell: return "Sell";
        default: return "Unknown";
    }
}

TransactionType Transaction::transactionTypeFromString(const std::string& typeStr) {
    if (typeStr == "Buy") return TransactionType::Buy;
    if (typeStr == "Sell") return TransactionType::Sell;
    
    return TransactionType::Buy; // По умолчанию покупка
}

nlohmann::json Transaction::toJson() const {
    nlohmann::json j;
    
    j["type"] = transactionTypeToString(type);
    j["quantity"] = quantity;
    j["price_per_share"] = pricePerShare;
    j["commission_rate"] = commissionRate;
    j["commission_amount"] = commissionAmount;
    j["total_cost"] = totalCost;
    j["transaction_day"] = transactionDay;
    j["executed"] = executed;
    j["status"] = status;
    
    auto companyPtr = company.lock();
    if (companyPtr) {
        j["company_ticker"] = companyPtr->getTicker();
    } else {
        j["company_ticker"] = "";
    }
    
    return j;
}

Transaction Transaction::fromJson(const nlohmann::json& json) {
    Transaction transaction;
    
    transaction.type = transactionTypeFromString(json["type"]);
    transaction.quantity = json["quantity"];
    transaction.pricePerShare = json["price_per_share"];
    transaction.commissionRate = json["commission_rate"];
    transaction.commissionAmount = json["commission_amount"];
    transaction.totalCost = json["total_cost"];
    transaction.transactionDay = json["transaction_day"];
    transaction.executed = json["executed"];
    transaction.status = json["status"];
    
    
    return transaction;
}

}