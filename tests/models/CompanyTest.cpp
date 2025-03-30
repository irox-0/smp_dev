#include <gtest/gtest.h>
#include "../../src/models/Company.hpp"
#include "../../src/utils/Date.hpp"
#include "../../src/core/Market.hpp"

using namespace StockMarketSimulator;

class CompanyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a sample company for testing
        dividendPolicy = DividendPolicy(2.0, 4); // $2.00 annual dividend paid quarterly
        testCompany = std::make_shared<Company>(
            "TestCompany",
            "TEST",
            "A company for testing",
            Sector::Technology,
            100.0, // Initial price
            0.5,   // Volatility
            dividendPolicy
        );
    }

    DividendPolicy dividendPolicy;
    std::shared_ptr<Company> testCompany;
};

// Test constructors and initialization
TEST_F(CompanyTest, ConstructorInitialization) {
    // Test default constructor
    Company defaultCompany;
    EXPECT_EQ(defaultCompany.getName(), "");
    EXPECT_EQ(defaultCompany.getTicker(), "");
    EXPECT_EQ(defaultCompany.getDescription(), "");
    EXPECT_EQ(defaultCompany.getSector(), Sector::Unknown);
    EXPECT_EQ(defaultCompany.getVolatility(), 0.0);
    EXPECT_NE(defaultCompany.getStock(), nullptr);

    // Test parameterized constructor
    EXPECT_EQ(testCompany->getName(), "TestCompany");
    EXPECT_EQ(testCompany->getTicker(), "TEST");
    EXPECT_EQ(testCompany->getDescription(), "A company for testing");
    EXPECT_EQ(testCompany->getSector(), Sector::Technology);
    EXPECT_EQ(testCompany->getVolatility(), 0.5);
    EXPECT_NE(testCompany->getStock(), nullptr);
    EXPECT_EQ(testCompany->getStock()->getCurrentPrice(), 100.0);
}

// Test getters and setters
TEST_F(CompanyTest, GettersAndSetters) {
    // Test getters
    EXPECT_EQ(testCompany->getName(), "TestCompany");
    EXPECT_EQ(testCompany->getTicker(), "TEST");
    EXPECT_EQ(testCompany->getDescription(), "A company for testing");
    EXPECT_EQ(testCompany->getSector(), Sector::Technology);
    EXPECT_EQ(testCompany->getSectorName(), "Technology");
    EXPECT_EQ(testCompany->getVolatility(), 0.5);

    // Test setters
    testCompany->setName("NewName");
    EXPECT_EQ(testCompany->getName(), "NewName");

    testCompany->setTicker("NEW");
    EXPECT_EQ(testCompany->getTicker(), "NEW");

    testCompany->setDescription("New description");
    EXPECT_EQ(testCompany->getDescription(), "New description");

    testCompany->setSector(Sector::Finance);
    EXPECT_EQ(testCompany->getSector(), Sector::Finance);
    EXPECT_EQ(testCompany->getSectorName(), "Finance");

    testCompany->setVolatility(0.7);
    EXPECT_EQ(testCompany->getVolatility(), 0.7);

    // Test financial setters
    testCompany->setFinancials(1000000.0, 15.0, 500000.0, 100000.0);
    EXPECT_EQ(testCompany->getMarketCap(), 1000000.0);
    EXPECT_EQ(testCompany->getPERatio(), 15.0);
    EXPECT_EQ(testCompany->getRevenue(), 500000.0);
    EXPECT_EQ(testCompany->getProfit(), 100000.0);
}

// Test dividend policy
TEST_F(CompanyTest, DividendPolicy) {
    const DividendPolicy& policy = testCompany->getDividendPolicy();

    // Test initial values
    EXPECT_EQ(policy.annualDividendRate, 2.0);
    EXPECT_EQ(policy.paymentFrequency, 4);
    EXPECT_EQ(policy.daysBetweenPayments, 91); // 365/4 = 91.25, truncated to 91

    // Test dividend amount calculation
    EXPECT_EQ(testCompany->calculateDividendAmount(), 0.5); // $2.00 / 4 = $0.50

    // Test dividend payment scheduling
    Date testDate(1, 4, 2023); // April 1, 2023

    // Initial state should not trigger a dividend
    EXPECT_FALSE(policy.shouldPayDividend(testDate));

    // Schedule next payment and verify
    DividendPolicy modifiedPolicy = policy;
    modifiedPolicy.scheduleNextPayment(testDate);
    Date expectedNextPayment(1, 7, 2023); // July 1, 2023 (91 days later)

    // Test shouldPayDividend before and after due date
    EXPECT_FALSE(modifiedPolicy.shouldPayDividend(Date(30, 6, 2023)));
    EXPECT_TRUE(modifiedPolicy.shouldPayDividend(Date(1, 7, 2023)));
    EXPECT_TRUE(modifiedPolicy.shouldPayDividend(Date(2, 7, 2023)));

    // Test processDividends
    // Set up a company with a scheduled dividend payment
    DividendPolicy scheduledPolicy(2.0, 4);
    Date currentDate(1, 3, 2023);
    Date nextPaymentDate(1, 4, 2023);

    // Manually set next payment date
    scheduledPolicy.nextPaymentDay = nextPaymentDate;

    Company companyWithDividend("DivCompany", "DIV", "Description", Sector::Technology,
                               100.0, 0.5, scheduledPolicy);

    // Verify no dividend due before payment date
    EXPECT_FALSE(companyWithDividend.processDividends(currentDate));

    // Verify dividend due on payment date
    EXPECT_TRUE(companyWithDividend.processDividends(nextPaymentDate));

    // Verify next payment is scheduled after processing
    const DividendPolicy& updatedPolicy = companyWithDividend.getDividendPolicy();
    Date expectedNewPaymentDate(1, 7, 2023); // April 1 + 91 days = July 1
    EXPECT_EQ(updatedPolicy.nextPaymentDay.getDay(), expectedNewPaymentDate.getDay());
    EXPECT_EQ(updatedPolicy.nextPaymentDay.getMonth(), expectedNewPaymentDate.getMonth());
    EXPECT_EQ(updatedPolicy.nextPaymentDay.getYear(), expectedNewPaymentDate.getYear());
}

// Test stock price updates
TEST_F(CompanyTest, StockPriceUpdates) {
    // Initial price should be 100.0
    EXPECT_EQ(testCompany->getStock()->getCurrentPrice(), 100.0);

    // Test updateStockPrice with market and sector trends
    double marketTrend = 0.05;  // 5% market increase
    double sectorTrend = 0.03;  // 3% sector increase

    testCompany->updateStockPrice(marketTrend, sectorTrend);

    // The exact price change depends on the implementation details, but it should increase
    EXPECT_GT(testCompany->getStock()->getCurrentPrice(), 100.0);

    // Test news impact - positive news
    double initialPrice = testCompany->getStock()->getCurrentPrice();
    double positiveNewsImpact = 0.02; // 2% increase

    testCompany->processNewsImpact(positiveNewsImpact);
    double afterPositiveNews = testCompany->getStock()->getCurrentPrice();

    // Price should increase approximately by the impact percentage
    EXPECT_GT(afterPositiveNews, initialPrice);

    // Test news impact - negative news
    double negativeNewsImpact = -0.03; // 3% decrease

    testCompany->processNewsImpact(negativeNewsImpact);
    double afterNegativeNews = testCompany->getStock()->getCurrentPrice();

    // Price should decrease approximately by the impact percentage
    EXPECT_LT(afterNegativeNews, afterPositiveNews);
}

// Test trading day operations
TEST_F(CompanyTest, TradingDayOperations) {
    // Get initial price
    double initialPrice = testCompany->getStock()->getCurrentPrice();

    // Test open and close trading day
    Date currentDate(1, 3, 2023);

    testCompany->openTradingDay(currentDate);
    // After opening the day, the open price should equal the current price
    EXPECT_EQ(testCompany->getStock()->getOpenPrice(), initialPrice);

    // Update price
    testCompany->processNewsImpact(0.01); // 1% increase

    // After price change, the open price should still be the initial price
    EXPECT_EQ(testCompany->getStock()->getOpenPrice(), initialPrice);
    // And current price should be different
    EXPECT_NE(testCompany->getStock()->getCurrentPrice(), initialPrice);

    // Close the trading day
    testCompany->closeTradingDay(currentDate);

    // After closing, the previous close should be the final price of the day
    EXPECT_EQ(testCompany->getStock()->getPreviousClosePrice(), testCompany->getStock()->getCurrentPrice());
}

// Test sector string conversion via getSectorName
TEST_F(CompanyTest, SectorStringConversion) {
    // Create companies with different sectors and test getSectorName
    Company techCompany("TechCo", "TCH", "Tech", Sector::Technology, 100.0, 0.5, DividendPolicy());
    Company energyCompany("EnergyCo", "NRG", "Energy", Sector::Energy, 100.0, 0.5, DividendPolicy());
    Company financeCompany("FinCo", "FIN", "Finance", Sector::Finance, 100.0, 0.5, DividendPolicy());
    Company consumerCompany("ConsCo", "CON", "Consumer", Sector::Consumer, 100.0, 0.5, DividendPolicy());
    Company mfgCompany("MfgCo", "MFG", "Manufacturing", Sector::Manufacturing, 100.0, 0.5, DividendPolicy());
    Company unknownCompany("UnkCo", "UNK", "Unknown", Sector::Unknown, 100.0, 0.5, DividendPolicy());

    // Test getSectorName returns the correct string for each sector
    EXPECT_EQ(techCompany.getSectorName(), "Technology");
    EXPECT_EQ(energyCompany.getSectorName(), "Energy");
    EXPECT_EQ(financeCompany.getSectorName(), "Finance");
    EXPECT_EQ(consumerCompany.getSectorName(), "Consumer");
    EXPECT_EQ(mfgCompany.getSectorName(), "Manufacturing");
    EXPECT_EQ(unknownCompany.getSectorName(), "Unknown");
}

// Test copy constructor and assignment operator
TEST_F(CompanyTest, CopyAndAssignment) {
    // Test copy constructor
    Company copiedCompany(*testCompany);

    EXPECT_EQ(copiedCompany.getName(), testCompany->getName());
    EXPECT_EQ(copiedCompany.getTicker(), testCompany->getTicker());
    EXPECT_EQ(copiedCompany.getDescription(), testCompany->getDescription());
    EXPECT_EQ(copiedCompany.getSector(), testCompany->getSector());
    EXPECT_EQ(copiedCompany.getVolatility(), testCompany->getVolatility());
    EXPECT_EQ(copiedCompany.getStock()->getCurrentPrice(), testCompany->getStock()->getCurrentPrice());

    // Test assignment operator
    Company assignedCompany;
    assignedCompany = *testCompany;

    EXPECT_EQ(assignedCompany.getName(), testCompany->getName());
    EXPECT_EQ(assignedCompany.getTicker(), testCompany->getTicker());
    EXPECT_EQ(assignedCompany.getDescription(), testCompany->getDescription());
    EXPECT_EQ(assignedCompany.getSector(), testCompany->getSector());
    EXPECT_EQ(assignedCompany.getVolatility(), testCompany->getVolatility());
    EXPECT_EQ(assignedCompany.getStock()->getCurrentPrice(), testCompany->getStock()->getCurrentPrice());
}

// Test JSON serialization and deserialization
TEST_F(CompanyTest, JsonSerialization) {
    // Serialize to JSON
    nlohmann::json json = testCompany->toJson();

    // Verify JSON structure
    EXPECT_EQ(json["name"], testCompany->getName());
    EXPECT_EQ(json["ticker"], testCompany->getTicker());
    EXPECT_EQ(json["description"], testCompany->getDescription());
    EXPECT_EQ(json["sector"], Market::sectorToString(testCompany->getSector()));
    EXPECT_EQ(json["volatility"], testCompany->getVolatility());

    // Deserialize from JSON
    std::shared_ptr<Company> deserializedCompany = Company::fromJson(json);

    // Verify deserialized company
    EXPECT_EQ(deserializedCompany->getName(), testCompany->getName());
    EXPECT_EQ(deserializedCompany->getTicker(), testCompany->getTicker());
    EXPECT_EQ(deserializedCompany->getDescription(), testCompany->getDescription());
    EXPECT_EQ(deserializedCompany->getSector(), testCompany->getSector());
    EXPECT_EQ(deserializedCompany->getVolatility(), testCompany->getVolatility());
    EXPECT_EQ(deserializedCompany->getStock()->getCurrentPrice(), testCompany->getStock()->getCurrentPrice());
}

// Test edge cases for volatility setting
TEST_F(CompanyTest, VolatilityEdgeCases) {
    // Test setting volatility to negative value (should clamp to 0.0)
    testCompany->setVolatility(-0.5);
    EXPECT_EQ(testCompany->getVolatility(), 0.0);

    // Test setting volatility to > 1.0 (should clamp to 1.0)
    testCompany->setVolatility(1.5);
    EXPECT_EQ(testCompany->getVolatility(), 1.0);

    // Test setting volatility to 0.0
    testCompany->setVolatility(0.0);
    EXPECT_EQ(testCompany->getVolatility(), 0.0);

    // Test setting volatility to 1.0
    testCompany->setVolatility(1.0);
    EXPECT_EQ(testCompany->getVolatility(), 1.0);
}

// Test DividendPolicy edge cases
TEST_F(CompanyTest, DividendPolicyEdgeCases) {
    // Test zero dividend rate
    DividendPolicy zeroDividendPolicy(0.0, 4);
    EXPECT_EQ(zeroDividendPolicy.calculateDividendAmount(), 0.0);
    EXPECT_FALSE(zeroDividendPolicy.shouldPayDividend(Date(1, 3, 2023)));

    // Test zero frequency (no dividends)
    DividendPolicy zeroFrequencyPolicy(2.0, 0);
    EXPECT_EQ(zeroFrequencyPolicy.calculateDividendAmount(), 0.0);
    EXPECT_FALSE(zeroFrequencyPolicy.shouldPayDividend(Date(1, 3, 2023)));

    // Test negative values (should be handled gracefully)
    DividendPolicy negativePolicy(-1.0, -2);
    EXPECT_EQ(negativePolicy.calculateDividendAmount(), 0.0);
    EXPECT_FALSE(negativePolicy.shouldPayDividend(Date(1, 3, 2023)));
}

// Test backward compatibility methods
TEST_F(CompanyTest, BackwardCompatibility) {
    // Test processDividends with day number
    DividendPolicy policy(2.0, 4);

    // Set up next payment day
    Date nextPaymentDate(1, 4, 2023);
    policy.nextPaymentDay = nextPaymentDate;

    Company company("TestCompany", "TEST", "Description", Sector::Technology,
                  100.0, 0.5, policy);

    // Convert dates to day numbers relative to March 1, 2023
    Date referenceDate(1, 3, 2023);
    int beforePaymentDay = Date(30, 3, 2023).toDayNumber(referenceDate);
    int paymentDay = nextPaymentDate.toDayNumber(referenceDate);

    // Test with day numbers
    EXPECT_FALSE(company.processDividends(beforePaymentDay));
    EXPECT_TRUE(company.processDividends(paymentDay));

    // Test other backward compatibility methods
    company.openTradingDay(); // No Date parameter
    company.closeTradingDay(); // No Date parameter

    // Verify that stock and company state are updated
    EXPECT_NE(company.getStock(), nullptr);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}