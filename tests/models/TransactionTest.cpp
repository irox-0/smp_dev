#include <gtest/gtest.h>
#include "models/Transaction.hpp"
#include "models/Company.hpp"

using namespace StockMarketSimulator;

class TransactionTest : public ::testing::Test {
protected:
    void SetUp() override {
        testCompany = std::make_shared<Company>(
            "Test Company", "TEST",
            "A company for testing transactions", Sector::Technology,
            100.0, 0.5, DividendPolicy(2.0, 4)
        );
        
        buyTransaction = std::make_unique<Transaction>(
            TransactionType::Buy, testCompany, 10, 100.0, 0.01, 1
        );
        
        sellTransaction = std::make_unique<Transaction>(
            TransactionType::Sell, testCompany, 5, 120.0, 0.01, 10
        );
    }
    
    std::shared_ptr<Company> testCompany;
    std::unique_ptr<Transaction> buyTransaction;
    std::unique_ptr<Transaction> sellTransaction;
};

TEST_F(TransactionTest, InitializationTest) {
    ASSERT_EQ(buyTransaction->getType(), TransactionType::Buy);
    ASSERT_EQ(buyTransaction->getQuantity(), 10);
    ASSERT_EQ(buyTransaction->getPricePerShare(), 100.0);
    ASSERT_EQ(buyTransaction->getCommissionRate(), 0.01);
    ASSERT_EQ(buyTransaction->getTransactionDay(), 1);
    ASSERT_FALSE(buyTransaction->isExecuted());
    
    auto company = buyTransaction->getCompany().lock();
    ASSERT_EQ(company->getTicker(), "TEST");
    
    ASSERT_EQ(sellTransaction->getType(), TransactionType::Sell);
    ASSERT_EQ(sellTransaction->getQuantity(), 5);
    ASSERT_EQ(sellTransaction->getPricePerShare(), 120.0);
    ASSERT_EQ(sellTransaction->getTransactionDay(), 10);
}

TEST_F(TransactionTest, CommissionCalculationTest) {
    double expectedCommission = 10 * 100.0 * 0.01;
    ASSERT_EQ(buyTransaction->getCommissionAmount(), expectedCommission);
    
    buyTransaction->setCommissionRate(0.02);
    expectedCommission = 10 * 100.0 * 0.02;
    ASSERT_EQ(buyTransaction->getCommissionAmount(), expectedCommission);
    
    buyTransaction->setCommissionRate(-0.01);
    ASSERT_EQ(buyTransaction->getCommissionRate(), 0.02);
    
    buyTransaction->setCommissionRate(0.2);
    ASSERT_EQ(buyTransaction->getCommissionRate(), 0.02);
}

TEST_F(TransactionTest, TotalCostCalculationTest) {
    double baseCost = 10 * 100.0;
    double commission = baseCost * 0.01;
    double expectedTotalCost = baseCost + commission;
    
    ASSERT_EQ(buyTransaction->getTotalCost(), expectedTotalCost);
    
    baseCost = 5 * 120.0;
    commission = baseCost * 0.01;
    expectedTotalCost = baseCost - commission;
    
    ASSERT_EQ(sellTransaction->getTotalCost(), expectedTotalCost);
    
    double staticTotal = Transaction::calculateTotalWithCommission(100.0, 10, 0.01);
    ASSERT_EQ(staticTotal, 10 * 100.0 * 1.01);
}

TEST_F(TransactionTest, ValidationTest) {
    ASSERT_TRUE(buyTransaction->validateTransaction(1100.0));
    ASSERT_FALSE(buyTransaction->validateTransaction(900.0));
    
    ASSERT_TRUE(sellTransaction->validateSellTransaction(10));
    ASSERT_FALSE(sellTransaction->validateSellTransaction(3));
    
    buyTransaction->execute();
    ASSERT_FALSE(buyTransaction->validateTransaction(1100.0));
    
    Transaction invalidTransaction(TransactionType::Buy, testCompany, 0, 100.0, 0.01, 1);
    ASSERT_FALSE(invalidTransaction.validateTransaction(1000.0));
}

TEST_F(TransactionTest, ExecutionTest) {
    ASSERT_FALSE(buyTransaction->isExecuted());
    buyTransaction->execute();
    ASSERT_TRUE(buyTransaction->isExecuted());
    
    ASSERT_THROW(buyTransaction->execute(), std::runtime_error);
}

TEST_F(TransactionTest, TransactionTypeConversionTest) {
    ASSERT_EQ(Transaction::transactionTypeToString(TransactionType::Buy), "Buy");
    ASSERT_EQ(Transaction::transactionTypeToString(TransactionType::Sell), "Sell");
    
    ASSERT_EQ(Transaction::transactionTypeFromString("Buy"), TransactionType::Buy);
    ASSERT_EQ(Transaction::transactionTypeFromString("Sell"), TransactionType::Sell);
    ASSERT_EQ(Transaction::transactionTypeFromString("Unknown"), TransactionType::Buy);
}

TEST_F(TransactionTest, SettersTest) {
    Transaction transaction;
    
    transaction.setType(TransactionType::Sell);
    ASSERT_EQ(transaction.getType(), TransactionType::Sell);
    
    transaction.setCompany(testCompany);
    ASSERT_EQ(transaction.getCompany().lock()->getTicker(), "TEST");
    
    transaction.setQuantity(20);
    ASSERT_EQ(transaction.getQuantity(), 20);
    
    transaction.setPricePerShare(150.0);
    ASSERT_EQ(transaction.getPricePerShare(), 150.0);
    
    transaction.setTransactionDay(5);
    ASSERT_EQ(transaction.getTransactionDay(), 5);
    
    transaction.setStatus("Test Status");
    ASSERT_EQ(transaction.getStatus(), "Test Status");
    
    transaction.setQuantity(-10);
    ASSERT_EQ(transaction.getQuantity(), 20);
    
    transaction.setPricePerShare(-50.0);
    ASSERT_EQ(transaction.getPricePerShare(), 150.0);
    
    transaction.setTransactionDay(-1);
    ASSERT_EQ(transaction.getTransactionDay(), 5);
}

TEST_F(TransactionTest, JsonSerializationTest) {
    nlohmann::json json = buyTransaction->toJson();
    
    ASSERT_EQ(json["type"], "Buy");
    ASSERT_EQ(json["quantity"], 10);
    ASSERT_EQ(json["price_per_share"], 100.0);
    ASSERT_EQ(json["commission_rate"], 0.01);
    ASSERT_EQ(json["commission_amount"], 10.0);
    ASSERT_EQ(json["transaction_day"], 1);
    ASSERT_EQ(json["executed"], false);
    ASSERT_EQ(json["company_ticker"], "TEST");
    
    Transaction restoredTransaction = Transaction::fromJson(json);
    
    ASSERT_EQ(restoredTransaction.getType(), TransactionType::Buy);
    ASSERT_EQ(restoredTransaction.getQuantity(), 10);
    ASSERT_EQ(restoredTransaction.getPricePerShare(), 100.0);
    ASSERT_EQ(restoredTransaction.getCommissionRate(), 0.01);
    ASSERT_EQ(restoredTransaction.getCommissionAmount(), 10.0);
    ASSERT_EQ(restoredTransaction.getTransactionDay(), 1);
    ASSERT_EQ(restoredTransaction.isExecuted(), false);
    
}