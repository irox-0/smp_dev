#include <gtest/gtest.h>
#include "models/Company.hpp"

using namespace StockMarketSimulator;

class DividendPolicyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create different dividend policies for testing
        noDividends = DividendPolicy(0.0, 0);
        annualDividends = DividendPolicy(5.0, 1);
        quarterlyDividends = DividendPolicy(4.0, 4);
        monthlyDividends = DividendPolicy(12.0, 12);
    }
    
    DividendPolicy noDividends;
    DividendPolicy annualDividends;
    DividendPolicy quarterlyDividends;
    DividendPolicy monthlyDividends;
};

TEST_F(DividendPolicyTest, InitializationTest) {
    ASSERT_EQ(noDividends.annualDividendRate, 0.0);
    ASSERT_EQ(noDividends.paymentFrequency, 0);
    ASSERT_EQ(noDividends.daysBetweenPayments, 0);
    
    ASSERT_EQ(annualDividends.annualDividendRate, 5.0);
    ASSERT_EQ(annualDividends.paymentFrequency, 1);
    ASSERT_EQ(annualDividends.daysBetweenPayments, 365);
    
    ASSERT_EQ(quarterlyDividends.annualDividendRate, 4.0);
    ASSERT_EQ(quarterlyDividends.paymentFrequency, 4);
    ASSERT_EQ(quarterlyDividends.daysBetweenPayments, 91);
    
    ASSERT_EQ(monthlyDividends.annualDividendRate, 12.0);
    ASSERT_EQ(monthlyDividends.paymentFrequency, 12);
    ASSERT_EQ(monthlyDividends.daysBetweenPayments, 30);
}

TEST_F(DividendPolicyTest, PaymentSchedulingTest) {
    ASSERT_EQ(quarterlyDividends.nextPaymentDay, 0);
    
    quarterlyDividends.scheduleNextPayment(10);
    ASSERT_EQ(quarterlyDividends.nextPaymentDay, 10 + 91);
    
    quarterlyDividends.scheduleNextPayment(quarterlyDividends.nextPaymentDay);
    ASSERT_EQ(quarterlyDividends.nextPaymentDay, 10 + 91 + 91);
}

TEST_F(DividendPolicyTest, ShouldPayDividendTest) {
    ASSERT_FALSE(noDividends.shouldPayDividend(100));
    
    annualDividends.nextPaymentDay = 365;
    quarterlyDividends.nextPaymentDay = 91;
    monthlyDividends.nextPaymentDay = 30;
    
    ASSERT_FALSE(annualDividends.shouldPayDividend(364));
    ASSERT_FALSE(quarterlyDividends.shouldPayDividend(90));
    ASSERT_FALSE(monthlyDividends.shouldPayDividend(29));
    
    ASSERT_TRUE(annualDividends.shouldPayDividend(365));
    ASSERT_TRUE(quarterlyDividends.shouldPayDividend(91));
    ASSERT_TRUE(monthlyDividends.shouldPayDividend(30));
    
    ASSERT_TRUE(annualDividends.shouldPayDividend(366));
    ASSERT_TRUE(quarterlyDividends.shouldPayDividend(92));
    ASSERT_TRUE(monthlyDividends.shouldPayDividend(31));
}

TEST_F(DividendPolicyTest, CalculateDividendAmountTest) {
    ASSERT_EQ(noDividends.calculateDividendAmount(), 0.0);
    ASSERT_EQ(annualDividends.calculateDividendAmount(), 5.0);
    ASSERT_NEAR(quarterlyDividends.calculateDividendAmount(), 1.0, 0.001);
    ASSERT_NEAR(monthlyDividends.calculateDividendAmount(), 1.0, 0.001);
}

TEST_F(DividendPolicyTest, JsonSerializationTest) {
    quarterlyDividends.nextPaymentDay = 100;
    
    nlohmann::json json = quarterlyDividends.toJson();
    
    ASSERT_EQ(json["annual_dividend_rate"], 4.0);
    ASSERT_EQ(json["payment_frequency"], 4);
    ASSERT_EQ(json["days_between_payments"], 91);
    ASSERT_EQ(json["next_payment_day"], 100);
    
    DividendPolicy newPolicy = DividendPolicy::fromJson(json);
    
    ASSERT_EQ(newPolicy.annualDividendRate, 4.0);
    ASSERT_EQ(newPolicy.paymentFrequency, 4);
    ASSERT_EQ(newPolicy.daysBetweenPayments, 91);
    ASSERT_EQ(newPolicy.nextPaymentDay, 100);
}