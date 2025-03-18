#include <gtest/gtest.h>
#include "models/Loan.hpp"

using namespace StockMarketSimulator;

class LoanTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем тестовый кредит с суммой 10000, ставкой 5%, сроком 30 дней, взятый в день 1
        testLoan = std::make_unique<Loan>(10000.0, 0.05, 30, 1);
        
        // Создаем кредит с пользовательскими параметрами
        customLoan = std::make_unique<Loan>(5000.0, 0.08, 90, 1, 0.002, "Custom Loan");
    }
    
    std::unique_ptr<Loan> testLoan;
    std::unique_ptr<Loan> customLoan;
};

TEST_F(LoanTest, InitializationTest) {
    // Проверка параметров стандартного кредита
    ASSERT_EQ(testLoan->getAmount(), 10000.0);
    ASSERT_EQ(testLoan->getInterestRate(), 0.05);
    ASSERT_EQ(testLoan->getDurationDays(), 30);
    ASSERT_EQ(testLoan->getTakenOnDay(), 1);
    ASSERT_EQ(testLoan->getDueDay(), 31);
    ASSERT_EQ(testLoan->getInterestAccrued(), 0.0);
    ASSERT_EQ(testLoan->getPenaltyRate(), 0.001);
    ASSERT_EQ(testLoan->getPenaltyAccrued(), 0.0);
    ASSERT_FALSE(testLoan->getIsPaid());
    
    // Проверка параметров кредита с пользовательскими настройками
    ASSERT_EQ(customLoan->getAmount(), 5000.0);
    ASSERT_EQ(customLoan->getInterestRate(), 0.08);
    ASSERT_EQ(customLoan->getDurationDays(), 90);
    ASSERT_EQ(customLoan->getTakenOnDay(), 1);
    ASSERT_EQ(customLoan->getDueDay(), 91);
    ASSERT_EQ(customLoan->getPenaltyRate(), 0.002);
    ASSERT_EQ(customLoan->getDescription(), "Custom Loan");
}

TEST_F(LoanTest, DailyInterestCalculationTest) {
    // Расчет дневного процента
    double dailyInterest = testLoan->calculateDailyInterest();
    double expectedDailyInterest = 10000.0 * (0.05 / 365.0);
    
    ASSERT_NEAR(dailyInterest, expectedDailyInterest, 0.001);
    
    // Начисление процентов за день
    testLoan->accrueInterest();
    ASSERT_NEAR(testLoan->getInterestAccrued(), expectedDailyInterest, 0.001);
    
    // Начисление процентов за несколько дней
    for (int i = 0; i < 10; i++) {
        testLoan->accrueInterest();
    }
    
    ASSERT_NEAR(testLoan->getInterestAccrued(), expectedDailyInterest * 11, 0.001);
}

TEST_F(LoanTest, OverdueTest) {
    // В день взятия кредита он не просрочен
    ASSERT_FALSE(testLoan->isOverdue(1));
    
    // В день погашения еще не просрочен
    ASSERT_FALSE(testLoan->isOverdue(31));
    
    // После дня погашения просрочен
    ASSERT_TRUE(testLoan->isOverdue(32));
    
    // Проверка начисления штрафа
    testLoan->update(32); // День после срока погашения
    
    double expectedInterest = testLoan->calculateDailyInterest();
    double expectedPenalty = testLoan->calculateDailyPenalty();
    
    ASSERT_NEAR(testLoan->getInterestAccrued(), expectedInterest, 0.001);
    ASSERT_NEAR(testLoan->getPenaltyAccrued(), expectedPenalty, 0.001);
    
    // Несколько дней просрочки
    for (int i = 0; i < 5; i++) {
        testLoan->update(33 + i);
    }
    
    // Проверка накопленных процентов и штрафов за 6 дней просрочки
    ASSERT_NEAR(testLoan->getInterestAccrued(), expectedInterest * 6, 0.001);
    ASSERT_NEAR(testLoan->getPenaltyAccrued(), expectedPenalty * 6, 0.001);
}

TEST_F(LoanTest, PaymentTest) {
    // Начисление процентов за несколько дней
    for (int i = 0; i < 10; i++) {
        testLoan->update(2 + i);
    }
    
    // Проверка суммы к погашению
    double totalDue = testLoan->getTotalDue();
    double expectedTotal = 10000.0 + testLoan->getInterestAccrued() + testLoan->getPenaltyAccrued();
    
    ASSERT_NEAR(totalDue, expectedTotal, 0.001);
    
    // Погашение кредита
    testLoan->markAsPaid();
    ASSERT_TRUE(testLoan->getIsPaid());
    
    // После погашения проценты не начисляются
    double interestBefore = testLoan->getInterestAccrued();
    testLoan->update(15);
    ASSERT_EQ(testLoan->getInterestAccrued(), interestBefore);
}

TEST_F(LoanTest, DaysRemainingTest) {
    // Проверка оставшихся дней
    ASSERT_EQ(testLoan->daysRemaining(1), 30);
    ASSERT_EQ(testLoan->daysRemaining(16), 15);
    ASSERT_EQ(testLoan->daysRemaining(31), 0);
    ASSERT_EQ(testLoan->daysRemaining(35), 0);
    
    // После погашения всегда 0 дней
    testLoan->markAsPaid();
    ASSERT_EQ(testLoan->daysRemaining(10), 0);
}

TEST_F(LoanTest, UpdateTest) {
    // Обновление в течение нескольких дней
    for (int day = 2; day <= 35; day++) {
        testLoan->update(day);
    }
    
    // 30 дней обычных процентов + 5 дней процентов и штрафов
    double expectedInterest = testLoan->calculateDailyInterest() * 34;
    double expectedPenalty = testLoan->calculateDailyPenalty() * 4; // 32-35 дни
    
    ASSERT_NEAR(testLoan->getInterestAccrued(), expectedInterest, 0.01);
    ASSERT_NEAR(testLoan->getPenaltyAccrued(), expectedPenalty, 0.01);
}

TEST_F(LoanTest, CustomLoanTest) {
    // Тест кредита с нестандартными параметрами
    double dailyInterest = customLoan->calculateDailyInterest();
    double expectedDailyInterest = 5000.0 * (0.08 / 365.0);
    
    ASSERT_NEAR(dailyInterest, expectedDailyInterest, 0.001);
    
    // Тест штрафа с нестандартной ставкой
    double dailyPenalty = customLoan->calculateDailyPenalty();
    double expectedDailyPenalty = 5000.0 * 0.002;
    
    ASSERT_NEAR(dailyPenalty, expectedDailyPenalty, 0.001);
}

TEST_F(LoanTest, StaticMethodTest) {
    // Проверка статического метода расчета общих процентов
    double totalInterest = Loan::calculateTotalInterest(10000.0, 0.05, 365);
    double expectedTotalInterest = 10000.0 * 0.05;
    
    ASSERT_NEAR(totalInterest, expectedTotalInterest, 0.001);
}

TEST_F(LoanTest, JsonSerializationTest) {
    // Начисление процентов за несколько дней
    for (int i = 0; i < 10; i++) {
        testLoan->update(2 + i);
    }
    
    // Сериализация в JSON
    nlohmann::json json = testLoan->toJson();
    
    // Проверка сериализованных данных
    ASSERT_EQ(json["amount"], 10000.0);
    ASSERT_EQ(json["interest_rate"], 0.05);
    ASSERT_EQ(json["duration_days"], 30);
    ASSERT_EQ(json["taken_on_day"], 1);
    ASSERT_EQ(json["due_day"], 31);
    ASSERT_NEAR(json["interest_accrued"].get<double>(), testLoan->getInterestAccrued(), 0.001);
    
    // Десериализация
    Loan restoredLoan = Loan::fromJson(json);
    
    // Проверка восстановленных данных
    ASSERT_EQ(restoredLoan.getAmount(), testLoan->getAmount());
    ASSERT_EQ(restoredLoan.getInterestRate(), testLoan->getInterestRate());
    ASSERT_EQ(restoredLoan.getDurationDays(), testLoan->getDurationDays());
    ASSERT_EQ(restoredLoan.getTakenOnDay(), testLoan->getTakenOnDay());
    ASSERT_EQ(restoredLoan.getDueDay(), testLoan->getDueDay());
    ASSERT_NEAR(restoredLoan.getInterestAccrued(), testLoan->getInterestAccrued(), 0.001);
    ASSERT_NEAR(restoredLoan.getPenaltyAccrued(), testLoan->getPenaltyAccrued(), 0.001);
    ASSERT_EQ(restoredLoan.getIsPaid(), testLoan->getIsPaid());
}

TEST_F(LoanTest, SettersTest) {
    // Проверка сеттеров
    testLoan->setAmount(15000.0);
    ASSERT_EQ(testLoan->getAmount(), 15000.0);
    
    testLoan->setInterestRate(0.07);
    ASSERT_EQ(testLoan->getInterestRate(), 0.07);
    
    testLoan->setDurationDays(60);
    ASSERT_EQ(testLoan->getDurationDays(), 60);
    ASSERT_EQ(testLoan->getDueDay(), 61); // 1 + 60
    
    testLoan->setPenaltyRate(0.003);
    ASSERT_EQ(testLoan->getPenaltyRate(), 0.003);
    
    testLoan->setDescription("Modified Loan");
    ASSERT_EQ(testLoan->getDescription(), "Modified Loan");
    
    // Проверка защиты от некорректных значений
    testLoan->setAmount(-1000.0);
    ASSERT_EQ(testLoan->getAmount(), 15000.0); // не должно измениться
    
    testLoan->setInterestRate(-0.05);
    ASSERT_EQ(testLoan->getInterestRate(), 0.07); // не должно измениться
    
    testLoan->setDurationDays(-30);
    ASSERT_EQ(testLoan->getDurationDays(), 60); // не должно измениться
}