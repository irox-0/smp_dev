#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace StockMarketSimulator {

// Класс для управления кредитами
class Loan {
private:
    double amount;
    double interestRate;
    int durationDays;
    int takenOnDay;
    int dueDay;
    double interestAccrued;
    double penaltyRate;
    double penaltyAccrued;
    bool isPaid;
    std::string description;

public:
    Loan();
    Loan(double amount, double interestRate, int durationDays, int takenOnDay);
    Loan(double amount, double interestRate, int durationDays, int takenOnDay,
         double penaltyRate, const std::string& description);

    double getAmount() const;
    double getInterestRate() const;
    int getDurationDays() const;
    int getTakenOnDay() const;
    int getDueDay() const;
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
    bool isOverdue(int currentDay) const;
    double getTotalDue() const;
    void markAsPaid();

    void update(int currentDay);

    int daysRemaining(int currentDay) const;

    static double calculateTotalInterest(double amount, double rate, int days);

    nlohmann::json toJson() const;
    static Loan fromJson(const nlohmann::json& json);
};

}
