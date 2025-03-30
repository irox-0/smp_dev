#include <gtest/gtest.h>
#include <memory>
#include "../../src/models/Loan.hpp"

using namespace StockMarketSimulator;

TEST(LoanTest, Constructor) {
    // Test default constructor
    Loan defaultLoan;
    EXPECT_EQ(defaultLoan.getAmount(), 0.0);
    EXPECT_EQ(defaultLoan.getInterestRate(), 0.0);
    EXPECT_EQ(defaultLoan.getDurationDays(), 0);
    EXPECT_FALSE(defaultLoan.getIsPaid());

    // Test parameterized constructor
    Date takenDate(15, 3, 2023);
    Loan loan(1000.0, 0.05, 30, takenDate);
    EXPECT_EQ(loan.getAmount(), 1000.0);
    EXPECT_EQ(loan.getInterestRate(), 0.05);
    EXPECT_EQ(loan.getDurationDays(), 30);
    EXPECT_EQ(loan.getTakenOnDate(), takenDate);

    // Check that due date is calculated correctly
    Date expectedDueDate(14, 4, 2023); // 30 days after March 15, 2023
    EXPECT_EQ(loan.getDueDate(), expectedDueDate);
    EXPECT_FALSE(loan.getIsPaid());
}

TEST(LoanTest, InterestCalculation) {
    Date takenDate(1, 1, 2023);
    Loan loan(1000.0, 0.10, 30, takenDate); // 10% annual interest, 30 days

    // Calculate expected daily interest: amount * (rate / 365)
    double expectedDailyInterest = 1000.0 * (0.10 / 365.0);
    EXPECT_DOUBLE_EQ(loan.calculateDailyInterest(), expectedDailyInterest);

    // Accrue interest
    loan.accrueInterest();
    EXPECT_DOUBLE_EQ(loan.getInterestAccrued(), expectedDailyInterest);

    // Accrue interest multiple times
    for (int i = 0; i < 10; i++) {
        loan.accrueInterest();
    }
    EXPECT_DOUBLE_EQ(loan.getInterestAccrued(), expectedDailyInterest * 11);
}

TEST(LoanTest, OverdueAndPenalty) {
    Date takenDate(1, 1, 2023);
    Loan loan(1000.0, 0.10, 10, takenDate); // 10-day loan

    // Check if loan is overdue
    Date beforeDue(10, 1, 2023); // Before due date
    EXPECT_FALSE(loan.isOverdue(beforeDue));

    Date afterDue(12, 1, 2023); // After due date
    EXPECT_TRUE(loan.isOverdue(afterDue));

    // Check penalty calculation
    double expectedDailyPenalty = 1000.0 * loan.getPenaltyRate();
    EXPECT_DOUBLE_EQ(loan.calculateDailyPenalty(), expectedDailyPenalty);

    // Accrue penalty
    loan.accruePenalty();
    EXPECT_DOUBLE_EQ(loan.getPenaltyAccrued(), expectedDailyPenalty);
}

TEST(LoanTest, TotalDueAndPaid) {
    Date takenDate(1, 1, 2023);
    Loan loan(1000.0, 0.10, 10, takenDate);

    // Initially total due is just the principal
    EXPECT_DOUBLE_EQ(loan.getTotalDue(), 1000.0);

    // Accrue some interest
    loan.accrueInterest();
    double expectedInterest = loan.calculateDailyInterest();
    EXPECT_DOUBLE_EQ(loan.getTotalDue(), 1000.0 + expectedInterest);

    // Accrue some penalty
    loan.accruePenalty();
    double expectedPenalty = loan.calculateDailyPenalty();
    EXPECT_DOUBLE_EQ(loan.getTotalDue(), 1000.0 + expectedInterest + expectedPenalty);

    // Mark loan as paid
    loan.markAsPaid();
    EXPECT_TRUE(loan.getIsPaid());

    // After marking as paid, further interest/penalty shouldn't accrue
    double totalDueBefore = loan.getTotalDue();
    loan.accrueInterest();
    loan.accruePenalty();
    EXPECT_DOUBLE_EQ(loan.getTotalDue(), totalDueBefore);
}

TEST(LoanTest, UpdateAndDaysRemaining) {
    Date takenDate(1, 1, 2023);
    Loan loan(1000.0, 0.10, 30, takenDate);

    // Update with current date
    Date currentDate(10, 1, 2023);
    loan.update(currentDate);
    double expectedInterest = loan.calculateDailyInterest();
    EXPECT_DOUBLE_EQ(loan.getInterestAccrued(), expectedInterest);

    // Days remaining
    EXPECT_EQ(loan.daysRemaining(currentDate), 21); // 30 - 9 days passed

    // Update with overdue date
    Date overdueDate(2, 2, 2023);
    loan.update(overdueDate);
    double expectedPenalty = loan.calculateDailyPenalty();
    EXPECT_GT(loan.getPenaltyAccrued(), 0.0);

    // No days remaining for overdue loan
    EXPECT_EQ(loan.daysRemaining(overdueDate), 0);
}

TEST(LoanTest, Serialization) {
    Date takenDate(1, 1, 2023);
    Loan loan(1000.0, 0.10, 30, takenDate, 0.002, "Test Loan");

    // Serialize to JSON
    nlohmann::json j = loan.toJson();

    // Deserialize from JSON
    Loan deserializedLoan = Loan::fromJson(j);

    // Check if deserialized loan matches original
    EXPECT_EQ(deserializedLoan.getAmount(), loan.getAmount());
    EXPECT_EQ(deserializedLoan.getInterestRate(), loan.getInterestRate());
    EXPECT_EQ(deserializedLoan.getDurationDays(), loan.getDurationDays());
    EXPECT_EQ(deserializedLoan.getTakenOnDate(), loan.getTakenOnDate());
    EXPECT_EQ(deserializedLoan.getDueDate(), loan.getDueDate());
    EXPECT_EQ(deserializedLoan.getDescription(), loan.getDescription());
    EXPECT_EQ(deserializedLoan.getPenaltyRate(), loan.getPenaltyRate());
}