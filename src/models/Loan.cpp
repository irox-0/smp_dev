#include "Loan.hpp"
#include <algorithm>

namespace StockMarketSimulator {

Loan::Loan()
    : amount(0.0),
      interestRate(0.0),
      durationDays(0),
      takenOnDate(),
      dueDate(),
      interestAccrued(0.0),
      penaltyRate(0.001),
      penaltyAccrued(0.0),
      isPaid(false),
      description("Standard Loan")
{
}

Loan::Loan(double amount, double interestRate, int durationDays, const Date& takenOnDate)
    : amount(amount),
      interestRate(interestRate),
      durationDays(durationDays),
      takenOnDate(takenOnDate),
      interestAccrued(0.0),
      penaltyRate(0.001),
      penaltyAccrued(0.0),
      isPaid(false),
      description("Standard Loan")
{
    dueDate = takenOnDate;
    dueDate.advanceDays(durationDays);
}

Loan::Loan(double amount, double interestRate, int durationDays, const Date& takenOnDate,
           double penaltyRate, const std::string& description)
    : amount(amount),
      interestRate(interestRate),
      durationDays(durationDays),
      takenOnDate(takenOnDate),
      interestAccrued(0.0),
      penaltyRate(penaltyRate),
      penaltyAccrued(0.0),
      isPaid(false),
      description(description)
{
    dueDate = takenOnDate;
    dueDate.advanceDays(durationDays);
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

Date Loan::getTakenOnDate() const {
    return takenOnDate;
}

Date Loan::getDueDate() const {
    return dueDate;
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
        this->dueDate = this->takenOnDate;
        this->dueDate.advanceDays(days);
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
    return amount * (interestRate / 7.0);
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

bool Loan::isOverdue(const Date& currentDate) const {
    return !isPaid && currentDate > dueDate;
}

double Loan::getTotalDue() const {
    return amount + interestAccrued + penaltyAccrued;
}

void Loan::markAsPaid() {
    isPaid = true;
}

void Loan::update(const Date& currentDate) {
    if (isPaid) {
        return;
    }

    accrueInterest();

    if (isOverdue(currentDate)) {
        accruePenalty();
    }
}

int Loan::daysRemaining(const Date& currentDate) const {
    if (isPaid || currentDate >= dueDate) {
        return 0;
    }
    return currentDate.daysBetween(dueDate);
}

double Loan::calculateTotalInterest(double amount, double rate, int days) {
    return amount * rate * (days / 7.0);
}

nlohmann::json Loan::toJson() const {
    nlohmann::json j;

    j["amount"] = amount;
    j["interest_rate"] = interestRate;
    j["duration_days"] = durationDays;
    j["taken_on_date"] = takenOnDate.toJson();
    j["due_date"] = dueDate.toJson();
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

    if (json.contains("taken_on_date")) {
        loan.takenOnDate = Date::fromJson(json["taken_on_date"]);
    } else if (json.contains("taken_on_day")) {
        int takenOnDay = json["taken_on_day"];
        loan.takenOnDate = Date::fromDayNumber(takenOnDay);
    }

    if (json.contains("due_date")) {
        loan.dueDate = Date::fromJson(json["due_date"]);
    } else if (json.contains("due_day")) {
        int dueDay = json["due_day"];
        loan.dueDate = Date::fromDayNumber(dueDay);
    } else {
        loan.dueDate = loan.takenOnDate;
        loan.dueDate.advanceDays(loan.durationDays);
    }

    loan.interestAccrued = json["interest_accrued"];
    loan.penaltyRate = json["penalty_rate"];
    loan.penaltyAccrued = json["penalty_accrued"];
    loan.isPaid = json["is_paid"];
    loan.description = json["description"];
    
    return loan;
}

}