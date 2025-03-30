#pragma once

#include <string>
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>
#include "../models/Portfolio.hpp"
#include "../models/Loan.hpp"
#include "../models/Transaction.hpp"
#include "../utils/Date.hpp"

namespace StockMarketSimulator {

class Market;

class Player {
private:
    std::string name;
    std::unique_ptr<Portfolio> portfolio;
    std::vector<Loan> loans;
    double marginAccountBalance;
    double marginUsed;
    double marginInterestRate;
    double marginAvailable;
    double marginRequirement;
    Date currentDate;
    std::weak_ptr<Market> market;

    void updateMarginAvailable();
    bool checkMarginCall();
    void accrueMarginInterest();

public:
    Player();
    Player(const std::string& name, double initialBalance = 10000.0);

    std::string getName() const;
    Portfolio* getPortfolio() const;
    double getMarginAccountBalance() const;
    double getMarginUsed() const;
    double getMarginAvailable() const;
    double getMarginRequirement() const;
    double getMarginInterestRate() const;
    double getTotalAssetValue() const;
    double getTotalLiabilities() const;
    double getNetWorth() const;
    const std::vector<Loan>& getLoans() const;
    Date getCurrentDate() const;
    int getCurrentDay() const;

    void setName(const std::string& name);
    void setMarket(std::weak_ptr<Market> market);
    void setCurrentDate(const Date& date);
    
    bool buyStock(std::shared_ptr<Company> company, int quantity, bool useMargin = false);
    bool sellStock(std::shared_ptr<Company> company, int quantity);
    
    bool takeLoan(double amount, double interestRate, int durationDays, const std::string& description = "Standard Loan");
    bool repayLoan(size_t loanIndex, double amount);
    bool depositCash(double amount);
    bool withdrawCash(double amount);
    
    bool depositToMarginAccount(double amount);
    bool withdrawFromMarginAccount(double amount);
    bool adjustMarginRequirement(double newRequirement);
    bool adjustMarginInterestRate(double newRate);
    
    void receiveDividends(std::shared_ptr<Company> company, double amountPerShare);
    
    void updateDailyState();
    void processLoans();
    void closeDay();
    void openDay();
    
    nlohmann::json toJson() const;
    static Player fromJson(const nlohmann::json& json, std::weak_ptr<Market> market);
};

}