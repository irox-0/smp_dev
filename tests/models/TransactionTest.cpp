#include <gtest/gtest.h>
#include <memory>
#include "../../src/models/Transaction.hpp"
#include "../../src/models/Company.hpp"

using namespace StockMarketSimulator;

class TransactionTest : public ::testing::Test {
protected:
    std::shared_ptr<Company> company;

    void SetUp() override {
        // Create a company for testing
        company = std::make_shared<Company>(
            "TestCompany", "TEST",
            "Test company for transaction tests",
            Sector::Technology,
            100.0, 0.5,
            DividendPolicy(2.0, 4)
        );
    }
};

TEST_F(TransactionTest, Constructor) {
    // Test default constructor
    Transaction defaultTx;
    EXPECT_EQ(defaultTx.getType(), TransactionType::Buy);
    EXPECT_EQ(defaultTx.getQuantity(), 0);
    EXPECT_FALSE(defaultTx.isExecuted());

    // Test parameterized constructor
    Date txDate(15, 3, 2023);
    Transaction tx(TransactionType::Buy, company, 10, 100.0, 0.01, txDate);
    EXPECT_EQ(tx.getType(), TransactionType::Buy);
    EXPECT_EQ(tx.getQuantity(), 10);
    EXPECT_EQ(tx.getPricePerShare(), 100.0);
    EXPECT_EQ(tx.getCommissionRate(), 0.01);
    EXPECT_EQ(tx.getTransactionDate(), txDate);
    EXPECT_FALSE(tx.isExecuted());

    // Check commission and total cost calculations
    double expectedCommission = 10 * 100.0 * 0.01;
    EXPECT_DOUBLE_EQ(tx.getCommissionAmount(), expectedCommission);
    EXPECT_DOUBLE_EQ(tx.getTotalCost(), 10 * 100.0 + expectedCommission);
}

TEST_F(TransactionTest, TypeAndCostCalculation) {
    Date txDate(15, 3, 2023);

    // Buy transaction
    Transaction buyTx(TransactionType::Buy, company, 10, 100.0, 0.01, txDate);
    double buyCommission = 10 * 100.0 * 0.01;
    EXPECT_DOUBLE_EQ(buyTx.getTotalCost(), 10 * 100.0 + buyCommission);

    // Sell transaction
    Transaction sellTx(TransactionType::Sell, company, 10, 100.0, 0.01, txDate);
    double sellCommission = 10 * 100.0 * 0.01;
    EXPECT_DOUBLE_EQ(sellTx.getTotalCost(), 10 * 100.0 - sellCommission);

    // Change transaction type
    buyTx.setType(TransactionType::Sell);
    EXPECT_EQ(buyTx.getType(), TransactionType::Sell);
    EXPECT_DOUBLE_EQ(buyTx.getTotalCost(), 10 * 100.0 - buyCommission);
}

TEST_F(TransactionTest, QuantityAndPriceUpdates) {
    Date txDate(15, 3, 2023);
    Transaction tx(TransactionType::Buy, company, 10, 100.0, 0.01, txDate);

    // Update quantity
    tx.setQuantity(20);
    EXPECT_EQ(tx.getQuantity(), 20);
    double newCommission = 20 * 100.0 * 0.01;
    EXPECT_DOUBLE_EQ(tx.getCommissionAmount(), newCommission);
    EXPECT_DOUBLE_EQ(tx.getTotalCost(), 20 * 100.0 + newCommission);

    // Update price
    tx.setPricePerShare(150.0);
    EXPECT_EQ(tx.getPricePerShare(), 150.0);
    newCommission = 20 * 150.0 * 0.01;
    EXPECT_DOUBLE_EQ(tx.getCommissionAmount(), newCommission);
    EXPECT_DOUBLE_EQ(tx.getTotalCost(), 20 * 150.0 + newCommission);

    // Update commission rate
    tx.setCommissionRate(0.02);
    EXPECT_EQ(tx.getCommissionRate(), 0.02);
    newCommission = 20 * 150.0 * 0.02;
    EXPECT_DOUBLE_EQ(tx.getCommissionAmount(), newCommission);
    EXPECT_DOUBLE_EQ(tx.getTotalCost(), 20 * 150.0 + newCommission);
}

TEST_F(TransactionTest, ValidationAndExecution) {
    Date txDate(15, 3, 2023);
    Transaction tx(TransactionType::Buy, company, 10, 100.0, 0.01, txDate);

    // Validate buy transaction with sufficient funds
    double availableFunds = 2000.0;
    EXPECT_TRUE(tx.validateTransaction(availableFunds));

    // Validate buy transaction with insufficient funds
    double insufficientFunds = 500.0;
    EXPECT_FALSE(tx.validateTransaction(insufficientFunds));

    // Validate sell transaction with sufficient shares
    Transaction sellTx(TransactionType::Sell, company, 5, 100.0, 0.01, txDate);
    int availableShares = 10;
    EXPECT_TRUE(sellTx.validateSellTransaction(availableShares));

    // Validate sell transaction with insufficient shares
    int insufficientShares = 3;
    EXPECT_FALSE(sellTx.validateSellTransaction(insufficientShares));

    // Execute transaction
    tx.execute();
    EXPECT_TRUE(tx.isExecuted());

    // Executing again should throw exception
    EXPECT_THROW(tx.execute(), std::runtime_error);
}

TEST_F(TransactionTest, Serialization) {
    Date txDate(15, 3, 2023);
    Transaction tx(TransactionType::Buy, company, 10, 100.0, 0.01, txDate);

    // Execute the transaction
    tx.execute();

    // Serialize to JSON
    nlohmann::json j = tx.toJson();

    // Deserialize from JSON
    Transaction deserializedTx = Transaction::fromJson(j);

    // Set the company back for the deserialized transaction
    deserializedTx.setCompany(company);

    // Check if deserialized transaction matches original
    EXPECT_EQ(deserializedTx.getType(), tx.getType());
    EXPECT_EQ(deserializedTx.getQuantity(), tx.getQuantity());
    EXPECT_EQ(deserializedTx.getPricePerShare(), tx.getPricePerShare());
    EXPECT_EQ(deserializedTx.getCommissionRate(), tx.getCommissionRate());
    EXPECT_EQ(deserializedTx.getCommissionAmount(), tx.getCommissionAmount());
    EXPECT_EQ(deserializedTx.getTotalCost(), tx.getTotalCost());
    EXPECT_EQ(deserializedTx.getTransactionDate(), tx.getTransactionDate());
    EXPECT_EQ(deserializedTx.isExecuted(), tx.isExecuted());
    EXPECT_EQ(deserializedTx.getStatus(), tx.getStatus());
}

TEST_F(TransactionTest, TransactionTypeConversion) {
    // Test string to type conversion
    EXPECT_EQ(Transaction::transactionTypeFromString("Buy"), TransactionType::Buy);
    EXPECT_EQ(Transaction::transactionTypeFromString("Sell"), TransactionType::Sell);
    EXPECT_EQ(Transaction::transactionTypeFromString("Invalid"), TransactionType::Buy); // Default

    // Test type to string conversion
    EXPECT_EQ(Transaction::transactionTypeToString(TransactionType::Buy), "Buy");
    EXPECT_EQ(Transaction::transactionTypeToString(TransactionType::Sell), "Sell");
}

TEST_F(TransactionTest, StaticHelperMethods) {
    // Test static calculation method
    double price = 100.0;
    int quantity = 10;
    double commissionRate = 0.01;
    double expected = 100.0 * 10 * (1.0 + 0.01);
    EXPECT_DOUBLE_EQ(Transaction::calculateTotalWithCommission(price, quantity, commissionRate), expected);
}