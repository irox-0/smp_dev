#include <gtest/gtest.h>
#include "models/Company.hpp"
#include "models/Stock.hpp"

using namespace StockMarketSimulator;

class CompanyTest : public ::testing::Test {
protected:
    void SetUp() override {
        testCompany = std::make_shared<Company>(
            "Tech Corporation", "TCH",
            "A leading technology corporation", Sector::Technology,
            50.0, 0.7, DividendPolicy(4.0, 4)
        );
    }

    std::shared_ptr<Company> testCompany;
};

TEST_F(CompanyTest, InitializationTest) {
    ASSERT_EQ(testCompany->getName(), "Tech Corporation");
    ASSERT_EQ(testCompany->getTicker(), "TCH");
    ASSERT_EQ(testCompany->getDescription(), "A leading technology corporation");
    ASSERT_EQ(testCompany->getSector(), Sector::Technology);
    ASSERT_EQ(testCompany->getSectorName(), "Technology");
    ASSERT_EQ(testCompany->getVolatility(), 0.7);

    const auto& policy = testCompany->getDividendPolicy();
    ASSERT_EQ(policy.annualDividendRate, 4.0);
    ASSERT_EQ(policy.paymentFrequency, 4);
    ASSERT_EQ(policy.daysBetweenPayments, 91);

    Stock* stock = testCompany->getStock();
    ASSERT_NE(stock, nullptr);
    ASSERT_EQ(stock->getCurrentPrice(), 50.0);
}

TEST_F(CompanyTest, StockUpdateTest) {
    testCompany->updateStockPrice(0.03, 0.02);

    Stock* stock = testCompany->getStock();
    ASSERT_NE(stock, nullptr);
    ASSERT_NE(stock->getCurrentPrice(), 50.0);

    double newPrice = stock->getCurrentPrice();
    ASSERT_GT(newPrice, 0.0);
}

TEST_F(CompanyTest, NewsImpactTest) {
    double initialPrice = testCompany->getStock()->getCurrentPrice();
    testCompany->processNewsImpact(0.1);

    double newPrice = testCompany->getStock()->getCurrentPrice();
    ASSERT_NEAR(newPrice, initialPrice * 1.1, 0.001);
}

TEST_F(CompanyTest, DividendProcessingTest) {
    auto policy = testCompany->getDividendPolicy();
    ASSERT_EQ(policy.nextPaymentDay, 0);

    ASSERT_FALSE(testCompany->processDividends(1));

    DividendPolicy updatedPolicy(4.0, 4);
    updatedPolicy.nextPaymentDay = 10;
    testCompany->setDividendPolicy(updatedPolicy);

    ASSERT_FALSE(testCompany->processDividends(9));

    ASSERT_TRUE(testCompany->processDividends(10));

    ASSERT_EQ(testCompany->getDividendPolicy().nextPaymentDay, 10 + 91);

    double expectedAmount = 4.0 / 4;
    ASSERT_NEAR(testCompany->calculateDividendAmount(), expectedAmount, 0.001);
}

TEST_F(CompanyTest, SectorOperationsTest) {
    ASSERT_EQ(testCompany->getSectorName(), "Technology");

    testCompany->setSector(Sector::Finance);
    ASSERT_EQ(testCompany->getSector(), Sector::Finance);
    ASSERT_EQ(testCompany->getSectorName(), "Finance");
}

TEST_F(CompanyTest, JsonSerializationTest) {
    testCompany->updateStockPrice(0.05, 0.03);
    testCompany->setFinancials(5000000.0, 15.2, 1000000.0, 330000.0);

    nlohmann::json json = testCompany->toJson();

    ASSERT_EQ(json["name"], "Tech Corporation");
    ASSERT_EQ(json["ticker"], "TCH");
    ASSERT_EQ(json["sector"], "Technology");
    ASSERT_EQ(json["volatility"], 0.7);
    ASSERT_EQ(json["market_cap"], 5000000.0);
    ASSERT_EQ(json["pe_ratio"], 15.2);
    ASSERT_EQ(json["revenue"], 1000000.0);
    ASSERT_EQ(json["profit"], 330000.0);
    ASSERT_TRUE(json.contains("stock"));
    ASSERT_TRUE(json.contains("dividend_policy"));

    auto newCompany = Company::fromJson(json);

    ASSERT_EQ(newCompany->getName(), "Tech Corporation");
    ASSERT_EQ(newCompany->getTicker(), "TCH");
    ASSERT_EQ(newCompany->getSector(), Sector::Technology);
    ASSERT_EQ(newCompany->getVolatility(), 0.7);
    ASSERT_EQ(newCompany->getMarketCap(), 5000000.0);
    ASSERT_EQ(newCompany->getPERatio(), 15.2);
    ASSERT_EQ(newCompany->getRevenue(), 1000000.0);
    ASSERT_EQ(newCompany->getProfit(), 330000.0);
    ASSERT_NE(newCompany->getStock(), nullptr);
}

TEST_F(CompanyTest, DividendPolicyTest) {
    DividendPolicy noPayment(0.0, 0);
    ASSERT_EQ(noPayment.daysBetweenPayments, 0);
    ASSERT_FALSE(noPayment.shouldPayDividend(10));

    DividendPolicy annual(3.0, 1);
    ASSERT_EQ(annual.daysBetweenPayments, 365);
    ASSERT_EQ(annual.calculateDividendAmount(), 3.0);

    DividendPolicy quarterly(4.0, 4);
    ASSERT_EQ(quarterly.daysBetweenPayments, 91);
    ASSERT_NEAR(quarterly.calculateDividendAmount(), 1.0, 0.001);

    DividendPolicy monthly(12.0, 12);
    ASSERT_EQ(monthly.daysBetweenPayments, 30);
    ASSERT_NEAR(monthly.calculateDividendAmount(), 1.0, 0.001);
}