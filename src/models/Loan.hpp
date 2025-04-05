#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "../utils/Date.hpp"

namespace StockMarketSimulator {

class Loan {
private:
    double amount;
    double interestRate;
    int durationDays;
    Date takenOnDate;
    Date dueDate;
    double interestAccrued;
    double penaltyRate;
    double penaltyAccrued;
    bool isPaid;
    std::string description;

public:
    Loan();
    Loan(double amount, double interestRate, int durationDays, const Date& takenOnDate);
    Loan(double amount, double interestRate, int durationDays, const Date& takenOnDate,
         double penaltyRate, const std::string& description);

    double getAmount() const;
    double getInterestRate() const;
    int getDurationDays() const;
    Date getTakenOnDate() const;
    Date getDueDate() const;
    double getInterestAccrued() const;
    double getPenaltyRate() const;
    double getPenaltyAccrued() const;
    bool getIsPaid() const;
    std::string getDescription() const;

    void setAmount(double amount);
    void setInterestRate(double rate);
    void setDurationDays(int days);
    void setPenaltyRate(double rate);
    void setDescription(const std::string& description);

    double calculateDailyInterest() const;
    double calculateDailyPenalty() const;
    void accrueInterest();
    void accruePenalty();
    bool isOverdue(const Date& currentDate) const;
    double getTotalDue() const;
    void markAsPaid();

    void update(const Date& currentDate);

    int daysRemaining(const Date& currentDate) const;

    static double calculateTotalInterest(double amount, double rate, int days);

    nlohmann::json toJson() const;
    static Loan fromJson(const nlohmann::json& json);
};

}