#include "Loan.hpp"
#include <algorithm>

namespace StockMarketSimulator {

Loan::Loan()
    : amount(0.0),
      interestRate(0.0),
      durationDays(0),
      takenOnDay(0),
      dueDay(0),
      interestAccrued(0.0),
      penaltyRate(0.001),
      penaltyAccrued(0.0),
      isPaid(false),
      description("Standard Loan")
{
}

Loan::Loan(double amount, double interestRate, int durationDays, int takenOnDay)
    : amount(amount),
      interestRate(interestRate),
      durationDays(durationDays),
      takenOnDay(takenOnDay),
      interestAccrued(0.0),
      penaltyRate(0.001),
      penaltyAccrued(0.0),
      isPaid(false),
      description("Standard Loan")
{
    dueDay = takenOnDay + durationDays;
}

Loan::Loan(double amount, double interestRate, int durationDays, int takenOnDay, 
           double penaltyRate, const std::string& description)
    : amount(amount),
      interestRate(interestRate),
      durationDays(durationDays),
      takenOnDay(takenOnDay),
      interestAccrued(0.0),
      penaltyRate(penaltyRate),
      penaltyAccrued(0.0),
      isPaid(false),
      description(description)
{
    dueDay = takenOnDay + durationDays;
}

double Loan::getAmount() const {
    return amount;
}

double Loan::getInterestRate() const {
    return interestRate;
}

int Loan::getDurationDays() const {
    return durationDays;
}

int Loan::getTakenOnDay() const {
    return takenOnDay;
}

int Loan::getDueDay() const {
    return dueDay;
}

double Loan::getInterestAccrued() const {
    return interestAccrued;
}

double Loan::getPenaltyRate() const {
    return penaltyRate;
}

double Loan::getPenaltyAccrued() const {
    return penaltyAccrued;
}

bool Loan::getIsPaid() const {
    return isPaid;
}

std::string Loan::getDescription() const {
    return description;
}

void Loan::setAmount(double amount) {
    if (amount > 0.0) {
        this->amount = amount;
    }
}

void Loan::setInterestRate(double rate) {
    if (rate >= 0.0) {
        this->interestRate = rate;
    }
}

void Loan::setDurationDays(int days) {
    if (days > 0) {
        this->durationDays = days;
        this->dueDay = this->takenOnDay + days;
    }
}

void Loan::setPenaltyRate(double rate) {
    if (rate >= 0.0) {
        this->penaltyRate = rate;
    }
}

void Loan::setDescription(const std::string& description) {
    this->description = description;
}

double Loan::calculateDailyInterest() const {
    if (isPaid) {
        return 0.0;
    }
    return amount * (interestRate / 365.0);
}

double Loan::calculateDailyPenalty() const {
    if (isPaid) {
        return 0.0;
    }
    return amount * penaltyRate;
}

void Loan::accrueInterest() {
    if (!isPaid) {
        interestAccrued += calculateDailyInterest();
    }
}

void Loan::accruePenalty() {
    if (!isPaid) {
        penaltyAccrued += calculateDailyPenalty();
    }
}

bool Loan::isOverdue(int currentDay) const {
    return !isPaid && currentDay > dueDay;
}

double Loan::getTotalDue() const {
    return amount + interestAccrued + penaltyAccrued;
}

void Loan::markAsPaid() {
    isPaid = true;
}

void Loan::update(int currentDay) {
    if (isPaid) {
        return;
    }
    
    accrueInterest();
    
    if (isOverdue(currentDay)) {
        accruePenalty();
    }
}

int Loan::daysRemaining(int currentDay) const {
    if (isPaid || currentDay >= dueDay) {
        return 0;
    }
    return dueDay - currentDay;
}

double Loan::calculateTotalInterest(double amount, double rate, int days) {
    return amount * rate * (days / 365.0);
}

nlohmann::json Loan::toJson() const {
    nlohmann::json j;
    
    j["amount"] = amount;
    j["interest_rate"] = interestRate;
    j["duration_days"] = durationDays;
    j["taken_on_day"] = takenOnDay;
    j["due_day"] = dueDay;
    j["interest_accrued"] = interestAccrued;
    j["penalty_rate"] = penaltyRate;
    j["penalty_accrued"] = penaltyAccrued;
    j["is_paid"] = isPaid;
    j["description"] = description;
    
    return j;
}

Loan Loan::fromJson(const nlohmann::json& json) {
    Loan loan;
    
    loan.amount = json["amount"];
    loan.interestRate = json["interest_rate"];
    loan.durationDays = json["duration_days"];
    loan.takenOnDay = json["taken_on_day"];
    loan.dueDay = json["due_day"];
    loan.interestAccrued = json["interest_accrued"];
    loan.penaltyRate = json["penalty_rate"];
    loan.penaltyAccrued = json["penalty_accrued"];
    loan.isPaid = json["is_paid"];
    loan.description = json["description"];
    
    return loan;
}

}