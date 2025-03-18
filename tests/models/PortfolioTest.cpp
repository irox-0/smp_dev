#include <gtest/gtest.h>
#include "models/Portfolio.hpp"
#include "models/Company.hpp"
#include "models/Stock.hpp"

using namespace StockMarketSimulator;

class PortfolioTest : public ::testing::Test {
protected:
    void SetUp() override {
        techCompany = std::make_shared<Company>(
            "Tech Corp", "TECH",
            "Technology Company", Sector::Technology,
            100.0, 0.5, DividendPolicy(2.0, 4)
        );

        energyCompany = std::make_shared<Company>(
            "Energy Corp", "ENRG",
            "Energy Company", Sector::Energy,
            50.0, 0.4, DividendPolicy(3.0, 4)
        );

        financeCompany = std::make_shared<Company>(
            "Finance Corp", "FIN",
            "Finance Company", Sector::Finance,
            75.0, 0.3, DividendPolicy(4.0, 4)
        );

        portfolio = std::make_unique<Portfolio>(10000.0);
    }

    std::shared_ptr<Company> techCompany;
    std::shared_ptr<Company> energyCompany;
    std::shared_ptr<Company> financeCompany;
    std::unique_ptr<Portfolio> portfolio;
};

TEST_F(PortfolioTest, InitializationTest) {
    ASSERT_EQ(portfolio->getInitialInvestment(), 10000.0);
    ASSERT_EQ(portfolio->getCashBalance(), 10000.0);
    ASSERT_EQ(portfolio->getTotalValue(), 10000.0);
    ASSERT_EQ(portfolio->getTotalStocksValue(), 0.0);
    ASSERT_EQ(portfolio->getPositions().size(), 0);
    ASSERT_EQ(portfolio->getHistory().size(), 0);
    ASSERT_EQ(portfolio->getTransactions().size(), 0);

    Portfolio emptyPortfolio;
    ASSERT_EQ(emptyPortfolio.getInitialInvestment(), 0.0);
    ASSERT_EQ(emptyPortfolio.getCashBalance(), 0.0);
    ASSERT_EQ(emptyPortfolio.getTotalValue(), 0.0);
}

TEST_F(PortfolioTest, BuyStockTest) {
    double initialBalance = portfolio->getCashBalance();
    std::cout << "\n=== BuyStockTest Debug Start ===\n";
    std::cout << "Initial balance: " << initialBalance << std::endl;

    bool result = portfolio->buyStock(techCompany, 20, 100.0, 0.01, 1);

    double expectedShareCost = 20 * 100.0;
    double expectedCommission = expectedShareCost * 0.01;
    double expectedTotalCost = expectedShareCost + expectedCommission;
    double expectedRemainingBalance = initialBalance - expectedTotalCost;

    std::cout << "Buy operation result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    std::cout << "Expected share cost: " << expectedShareCost << std::endl;
    std::cout << "Expected commission: " << expectedCommission << std::endl;
    std::cout << "Expected total cost: " << expectedTotalCost << std::endl;
    std::cout << "Expected remaining balance: " << expectedRemainingBalance << std::endl;
    std::cout << "Actual balance after buy: " << portfolio->getCashBalance() << std::endl;
    std::cout << "Balance difference: " << (portfolio->getCashBalance() - expectedRemainingBalance) << std::endl;

    bool hasPosition = portfolio->hasPosition("TECH");
    std::cout << "Has position for TECH: " << (hasPosition ? "YES" : "NO") << std::endl;

    if (hasPosition) {
        const PortfolioPosition* position = portfolio->getPosition("TECH");
        std::cout << "Position quantity: " << position->quantity << std::endl;
        std::cout << "Position avg price: " << position->averagePurchasePrice << std::endl;
        std::cout << "Position total cost: " << position->totalCost << std::endl;
        std::cout << "Position current value: " << position->currentValue << std::endl;
    }

    std::cout << "Transaction count: " << portfolio->getTransactions().size() << std::endl;

    if (!portfolio->getTransactions().empty()) {
        const auto& transaction = portfolio->getTransactions()[0];
        std::cout << "Transaction type: " << Transaction::transactionTypeToString(transaction.getType()) << std::endl;
        std::cout << "Transaction quantity: " << transaction.getQuantity() << std::endl;
        std::cout << "Transaction price: " << transaction.getPricePerShare() << std::endl;
        std::cout << "Transaction commission: " << transaction.getCommissionRate() << std::endl;
        std::cout << "Transaction total cost: " << transaction.getTotalCost() << std::endl;
    }

    std::cout << "\n=== Standard Assert Checks ===\n";
    std::cout << "ASSERT_TRUE(result) will " << (result ? "PASS" : "FAIL") << std::endl;
    std::cout << "ASSERT_NEAR(portfolio->getCashBalance(), expectedRemainingBalance, 0.01) will "
              << (std::abs(portfolio->getCashBalance() - expectedRemainingBalance) <= 0.01 ? "PASS" : "FAIL") << std::endl;
    std::cout << "ASSERT_EQ(portfolio->getPositions().size(), 1) will "
              << (portfolio->getPositions().size() == 1 ? "PASS" : "FAIL") << std::endl;
    std::cout << "=== BuyStockTest Debug End ===\n\n";

    ASSERT_TRUE(result);
    ASSERT_NEAR(portfolio->getCashBalance(), expectedRemainingBalance, 0.01);
    ASSERT_EQ(portfolio->getPositions().size(), 1);
    ASSERT_TRUE(portfolio->hasPosition("TECH"));
    ASSERT_EQ(portfolio->getPositionQuantity("TECH"), 20);
}

TEST_F(PortfolioTest, SellStockTest) {
    portfolio->buyStock(techCompany, 50, 100.0, 0.01, 1);
    double balanceAfterPurchase = portfolio->getCashBalance();

    bool result = portfolio->sellStock(techCompany, 20, 120.0, 0.01, 2);
    ASSERT_TRUE(result);

    ASSERT_EQ(portfolio->getPositionQuantity("TECH"), 30);

    double sellValue = 20 * 120.0;
    double commission = sellValue * 0.01;
    double expectedBalance = balanceAfterPurchase + (sellValue - commission);
    ASSERT_NEAR(portfolio->getCashBalance(), expectedBalance, 0.01);

    ASSERT_EQ(portfolio->getTransactions().size(), 2);
    ASSERT_EQ(portfolio->getTransactions()[1].getType(), TransactionType::Sell);
    ASSERT_EQ(portfolio->getTransactions()[1].getQuantity(), 20);

    result = portfolio->sellStock(techCompany, 30, 110.0, 0.01, 3);
    ASSERT_TRUE(result);

    ASSERT_FALSE(portfolio->hasPosition("TECH"));
    ASSERT_EQ(portfolio->getPositions().size(), 0);

    result = portfolio->sellStock(techCompany, 10, 100.0, 0.01, 4);
    ASSERT_FALSE(result);

    portfolio->buyStock(energyCompany, 5, 50.0, 0.01, 5);
    result = portfolio->sellStock(energyCompany, 10, 55.0, 0.01, 6);
    ASSERT_FALSE(result);
    ASSERT_EQ(portfolio->getPositionQuantity("ENRG"), 5);
}

TEST_F(PortfolioTest, PositionUpdateTest) {
    portfolio->buyStock(techCompany, 20, 100.0, 0.01, 1);

    techCompany->getStock()->updatePrice(120.0);

    portfolio->updatePositionValues();

    const PortfolioPosition* position = portfolio->getPosition("TECH");
    ASSERT_NE(position, nullptr);

    double expectedCurrentValue = 20 * 120.0;
    ASSERT_NEAR(position->currentValue, expectedCurrentValue, 0.01);

    double totalCost = 20 * 100.0;
    double expectedProfitLoss = expectedCurrentValue - totalCost;
    double expectedProfitLossPercent = (expectedProfitLoss / totalCost) * 100.0;

    ASSERT_NEAR(position->unrealizedProfitLoss, expectedProfitLoss, 0.01);
    ASSERT_NEAR(position->unrealizedProfitLossPercent, expectedProfitLossPercent, 0.01);

    double expectedTotalValue = portfolio->getCashBalance() + expectedCurrentValue;
    ASSERT_NEAR(portfolio->getTotalValue(), expectedTotalValue, 0.01);
}

TEST_F(PortfolioTest, CloseDayOpenDayTest) {
    portfolio->buyStock(techCompany, 20, 100.0, 0.01, 1);
    techCompany->getStock()->updatePrice(120.0);
    portfolio->updatePositionValues();

    portfolio->closeDay(1);
    ASSERT_EQ(portfolio->getHistory().size(), 1);

    const auto& historyEntry = portfolio->getHistory()[0];
    ASSERT_EQ(historyEntry.day, 1);
    ASSERT_NEAR(historyEntry.totalValue, portfolio->getTotalValue(), 0.01);
    ASSERT_NEAR(historyEntry.cashBalance, portfolio->getCashBalance(), 0.01);

    portfolio->openDay();
    ASSERT_NEAR(portfolio->getPreviousDayValue(), portfolio->getTotalValue(), 0.01);

    techCompany->getStock()->updatePrice(130.0);
    portfolio->updatePositionValues();

    double dayChange = portfolio->getDayChangeAmount();
    double dayChangePercent = portfolio->getDayChangePercent();

    double expectedChange = 20 * (130.0 - 120.0);
    double expectedChangePercent = (expectedChange / portfolio->getPreviousDayValue()) * 100.0;

    ASSERT_NEAR(dayChange, expectedChange, 0.01);
    ASSERT_NEAR(dayChangePercent, expectedChangePercent, 0.1);
}

TEST_F(PortfolioTest, DividendsTest) {
    portfolio->buyStock(techCompany, 50, 100.0, 0.01, 1);
    double balanceBeforeDividends = portfolio->getCashBalance();

    double dividendPerShare = 0.5;
    portfolio->receiveDividends(techCompany, dividendPerShare);

    double expectedDividend = 50 * dividendPerShare;
    ASSERT_NEAR(portfolio->getCashBalance(), balanceBeforeDividends + expectedDividend, 0.01);
    ASSERT_NEAR(portfolio->getTotalDividendsReceived(), expectedDividend, 0.01);

    double expectedTotalValue = balanceBeforeDividends + expectedDividend + portfolio->getTotalStocksValue();
    ASSERT_NEAR(portfolio->getTotalValue(), expectedTotalValue, 0.01);

    double previousBalance = portfolio->getCashBalance();
    portfolio->receiveDividends(energyCompany, 1.0);
    ASSERT_EQ(portfolio->getCashBalance(), previousBalance);
}

TEST_F(PortfolioTest, CashOperationsTest) {
    double initialBalance = portfolio->getCashBalance();

    portfolio->depositCash(5000.0);
    ASSERT_NEAR(portfolio->getCashBalance(), initialBalance + 5000.0, 0.01);
    ASSERT_NEAR(portfolio->getInitialInvestment(), 15000.0, 0.01);
    ASSERT_NEAR(portfolio->getTotalValue(), initialBalance + 5000.0, 0.01);

    bool result = portfolio->withdrawCash(2000.0);
    ASSERT_TRUE(result);
    ASSERT_NEAR(portfolio->getCashBalance(), initialBalance + 5000.0 - 2000.0, 0.01);
    ASSERT_NEAR(portfolio->getTotalValue(), initialBalance + 5000.0 - 2000.0, 0.01);

    double currentBalance = portfolio->getCashBalance();
    result = portfolio->withdrawCash(currentBalance + 1000.0);
    ASSERT_FALSE(result);
    ASSERT_NEAR(portfolio->getCashBalance(), currentBalance, 0.01);

    portfolio->depositCash(-1000.0);
    ASSERT_NEAR(portfolio->getCashBalance(), currentBalance, 0.01);

    result = portfolio->withdrawCash(-500.0);
    ASSERT_FALSE(result);
    ASSERT_NEAR(portfolio->getCashBalance(), currentBalance, 0.01);
}

TEST_F(PortfolioTest, PortfolioPerformanceTest) {
    std::cout << "\n=== PortfolioPerformanceTest Debug Start ===\n";

    std::cout << "Initial investment: " << portfolio->getInitialInvestment() << std::endl;
    std::cout << "Initial total return: " << portfolio->getTotalReturn() << std::endl;
    std::cout << "Initial percent return: " << portfolio->getTotalReturnPercent() << std::endl;

    std::cout << "\n--- Making purchases ---\n";
    bool buyResult1 = portfolio->buyStock(techCompany, 30, 100.0, 0.01, 1);
    std::cout << "Buy Tech result: " << (buyResult1 ? "SUCCESS" : "FAILED") << std::endl;
    std::cout << "Balance after Tech buy: " << portfolio->getCashBalance() << std::endl;

    bool buyResult2 = portfolio->buyStock(energyCompany, 20, 50.0, 0.01, 1);
    std::cout << "Buy Energy result: " << (buyResult2 ? "SUCCESS" : "FAILED") << std::endl;
    std::cout << "Balance after Energy buy: " << portfolio->getCashBalance() << std::endl;

    std::cout << "\n--- Closing day 1 ---\n";
    portfolio->closeDay(1);
    std::cout << "History size after day 1: " << portfolio->getHistory().size() << std::endl;

    std::cout << "\n--- Updating stock prices ---\n";
    double oldTechPrice = techCompany->getStock()->getCurrentPrice();
    double oldEnergyPrice = energyCompany->getStock()->getCurrentPrice();

    techCompany->getStock()->updatePrice(110.0);
    energyCompany->getStock()->updatePrice(55.0);

    std::cout << "Tech price changed: " << oldTechPrice << " -> " << techCompany->getStock()->getCurrentPrice() << std::endl;
    std::cout << "Energy price changed: " << oldEnergyPrice << " -> " << energyCompany->getStock()->getCurrentPrice() << std::endl;

    std::cout << "\n--- Updating portfolio and closing day 2 ---\n";
    portfolio->updatePositionValues();
    double portfolioValueBeforeDay2Close = portfolio->getTotalValue();
    std::cout << "Portfolio value before closing day 2: " << portfolioValueBeforeDay2Close << std::endl;

    portfolio->closeDay(2);
    std::cout << "History size after day 2: " << portfolio->getHistory().size() << std::endl;

    std::cout << "\n--- Opening day 3 ---\n";
    portfolio->openDay();
    std::cout << "Previous day value: " << portfolio->getPreviousDayValue() << std::endl;

    std::cout << "\n--- Calculations for validation ---\n";

    double techStockValue = 30 * 110.0;
    double energyStockValue = 20 * 55.0;
    double totalStocksValue = techStockValue + energyStockValue;

    std::cout << "Tech value: " << techStockValue << std::endl;
    std::cout << "Energy value: " << energyStockValue << std::endl;
    std::cout << "Total stocks value: " << totalStocksValue << std::endl;
    std::cout << "Actual portfolio stocks value: " << portfolio->getTotalStocksValue() << std::endl;

    double techInvestment = 30 * 100.0;
    double energyInvestment = 20 * 50.0;
    double techCommission = techInvestment * 0.01;
    double energyCommission = energyInvestment * 0.01;
    double totalInvested = techInvestment + energyInvestment;
    double totalCommission = techCommission + energyCommission;
    double totalInvestedWithCommission = totalInvested + totalCommission;

    std::cout << "Tech investment: " << techInvestment << " + commission: " << techCommission << std::endl;
    std::cout << "Energy investment: " << energyInvestment << " + commission: " << energyCommission << std::endl;
    std::cout << "Total invested: " << totalInvested << " + total commission: " << totalCommission << std::endl;

    double expectedReturn = portfolio->getTotalValue() - portfolio->getInitialInvestment();
    double expectedReturnPercent = (expectedReturn / portfolio->getInitialInvestment()) * 100.0;

    std::cout << "Initial investment: " << portfolio->getInitialInvestment() << std::endl;
    std::cout << "Total value: " << portfolio->getTotalValue() << std::endl;
    std::cout << "Cash balance: " << portfolio->getCashBalance() << std::endl;
    std::cout << "Expected return: " << expectedReturn << std::endl;
    std::cout << "Expected return %: " << expectedReturnPercent << std::endl;
    std::cout << "Actual return: " << portfolio->getTotalReturn() << std::endl;
    std::cout << "Actual return %: " << portfolio->getTotalReturnPercent() << std::endl;
    std::cout << "Return difference: " << (portfolio->getTotalReturn() - expectedReturn) << std::endl;
    std::cout << "Return % difference: " << (portfolio->getTotalReturnPercent() - expectedReturnPercent) << std::endl;

    double periodReturn = portfolio->getPeriodReturn(1);
    double periodReturnPercent = portfolio->getPeriodReturnPercent(1);
    double lastDayValue = portfolio->getHistory().size() > 0
        ? portfolio->getHistory()[portfolio->getHistory().size() - 2].totalValue : 0;

    std::cout << "\n--- Period return check ---\n";
    std::cout << "Last day value: " << lastDayValue << std::endl;
    std::cout << "Current value: " << portfolio->getTotalValue() << std::endl;
    std::cout << "Expected period return: " << (portfolio->getTotalValue() - lastDayValue) << std::endl;
    std::cout << "Actual period return: " << periodReturn << std::endl;
    std::cout << "Expected period return %: " << ((portfolio->getTotalValue() - lastDayValue) / lastDayValue * 100.0) << std::endl;
    std::cout << "Actual period return %: " << periodReturnPercent << std::endl;

    std::cout << "\n=== Standard Assert Checks ===\n";
    double totalReturnDiff = std::abs(portfolio->getTotalReturn() - expectedReturn);
    double totalReturnPercentDiff = std::abs(portfolio->getTotalReturnPercent() - expectedReturnPercent);

    std::cout << "ASSERT_NEAR(portfolio->getTotalReturn(), expectedReturn, 1.0) will "
              << (totalReturnDiff <= 1.0 ? "PASS" : "FAIL") << " (diff = " << totalReturnDiff << ")" << std::endl;
    std::cout << "ASSERT_NEAR(portfolio->getTotalReturnPercent(), expectedReturnPercent, 0.1) will "
              << (totalReturnPercentDiff <= 0.1 ? "PASS" : "FAIL") << " (diff = " << totalReturnPercentDiff << ")" << std::endl;

    std::cout << "=== PortfolioPerformanceTest Debug End ===\n\n";

    ASSERT_NEAR(portfolio->getTotalReturn(), expectedReturn, 1.0);
    ASSERT_NEAR(portfolio->getTotalReturnPercent(), expectedReturnPercent, 0.1);
}
TEST_F(PortfolioTest, SectorAllocationTest) {
    portfolio->buyStock(techCompany, 20, 100.0, 0.01, 1);      // Technology
    portfolio->buyStock(energyCompany, 30, 50.0, 0.01, 1);     // Energy
    portfolio->buyStock(financeCompany, 10, 75.0, 0.01, 1);    // Finance

    double techValue = 20 * techCompany->getStock()->getCurrentPrice();
    double energyValue = 30 * energyCompany->getStock()->getCurrentPrice();
    double financeValue = 10 * financeCompany->getStock()->getCurrentPrice();
    double totalStocksValue = techValue + energyValue + financeValue;

    std::map<Sector, double> sectorAllocation = portfolio->getSectorAllocation();

    ASSERT_NEAR(sectorAllocation[Sector::Technology], techValue, 0.01);
    ASSERT_NEAR(sectorAllocation[Sector::Energy], energyValue, 0.01);
    ASSERT_NEAR(sectorAllocation[Sector::Finance], financeValue, 0.01);

    double techPercent = portfolio->getSectorAllocationPercent(Sector::Technology);
    double energyPercent = portfolio->getSectorAllocationPercent(Sector::Energy);
    double financePercent = portfolio->getSectorAllocationPercent(Sector::Finance);

    ASSERT_NEAR(techPercent, (techValue / totalStocksValue) * 100.0, 0.1);
    ASSERT_NEAR(energyPercent, (energyValue / totalStocksValue) * 100.0, 0.1);
    ASSERT_NEAR(financePercent, (financeValue / totalStocksValue) * 100.0, 0.1);

    double consumerPercent = portfolio->getSectorAllocationPercent(Sector::Consumer);
    ASSERT_NEAR(consumerPercent, 0.0, 0.01);
}

TEST_F(PortfolioTest, ValueHistoryTest) {
    portfolio->buyStock(techCompany, 20, 100.0, 0.01, 1);
    portfolio->closeDay(1);

    techCompany->getStock()->updatePrice(110.0);
    portfolio->updatePositionValues();
    portfolio->closeDay(2);

    techCompany->getStock()->updatePrice(105.0);
    portfolio->updatePositionValues();
    portfolio->closeDay(3);

    std::vector<double> valueHistory = portfolio->getValueHistory();

    ASSERT_EQ(valueHistory.size(), 3);
    ASSERT_NEAR(valueHistory[0], portfolio->getHistory()[0].totalValue, 0.01);
    ASSERT_NEAR(valueHistory[1], portfolio->getHistory()[1].totalValue, 0.01);
    ASSERT_NEAR(valueHistory[2], portfolio->getHistory()[2].totalValue, 0.01);
}

TEST_F(PortfolioTest, JsonSerializationTest) {
    portfolio->buyStock(techCompany, 20, 100.0, 0.01, 1);
    portfolio->buyStock(energyCompany, 30, 50.0, 0.01, 2);

    techCompany->getStock()->updatePrice(110.0);
    energyCompany->getStock()->updatePrice(55.0);

    portfolio->updatePositionValues();
    portfolio->closeDay(1);
    portfolio->closeDay(2);

    nlohmann::json json = portfolio->toJson();

    ASSERT_EQ(json["cash_balance"], portfolio->getCashBalance());
    ASSERT_EQ(json["total_value"], portfolio->getTotalValue());
    ASSERT_EQ(json["initial_investment"], portfolio->getInitialInvestment());

    ASSERT_EQ(json["positions"].size(), 2);

    ASSERT_EQ(json["history"].size(), 2);

    ASSERT_EQ(json["transactions"].size(), 2);

    std::vector<std::shared_ptr<Company>> companies = {
        techCompany, energyCompany, financeCompany
    };

    Portfolio restoredPortfolio = Portfolio::fromJson(json, companies);

    ASSERT_NEAR(restoredPortfolio.getCashBalance(), portfolio->getCashBalance(), 0.01);
    ASSERT_NEAR(restoredPortfolio.getTotalValue(), portfolio->getTotalValue(), 0.01);
    ASSERT_NEAR(restoredPortfolio.getInitialInvestment(), portfolio->getInitialInvestment(), 0.01);

    ASSERT_EQ(restoredPortfolio.getPositions().size(), portfolio->getPositions().size());
    ASSERT_TRUE(restoredPortfolio.hasPosition("TECH"));
    ASSERT_TRUE(restoredPortfolio.hasPosition("ENRG"));
    ASSERT_EQ(restoredPortfolio.getPositionQuantity("TECH"), 20);
    ASSERT_EQ(restoredPortfolio.getPositionQuantity("ENRG"), 30);

    ASSERT_EQ(restoredPortfolio.getHistory().size(), portfolio->getHistory().size());

    ASSERT_EQ(restoredPortfolio.getTransactions().size(), portfolio->getTransactions().size());
}

TEST_F(PortfolioTest, EdgeCasesTest) {
    Portfolio emptyPortfolio;
    ASSERT_NEAR(emptyPortfolio.getTotalReturn(), 0.0, 0.01);
    ASSERT_NEAR(emptyPortfolio.getTotalReturnPercent(), 0.0, 0.01);
    ASSERT_NEAR(emptyPortfolio.getDayChangeAmount(), 0.0, 0.01);
    ASSERT_NEAR(emptyPortfolio.getDayChangePercent(), 0.0, 0.01);

    ASSERT_FALSE(portfolio->buyStock(techCompany, 0, 100.0, 0.01, 1));
    ASSERT_FALSE(portfolio->buyStock(techCompany, -10, 100.0, 0.01, 1));
    ASSERT_FALSE(portfolio->buyStock(techCompany, 10, 0.0, 0.01, 1));
    ASSERT_FALSE(portfolio->buyStock(techCompany, 10, -50.0, 0.01, 1));

    const PortfolioPosition* nullPosition = portfolio->getPosition("NONEXISTENT");
    ASSERT_EQ(nullPosition, nullptr);

    ASSERT_NEAR(portfolio->getPeriodReturn(10), portfolio->getTotalReturn(), 0.01);
    ASSERT_NEAR(portfolio->getPeriodReturnPercent(10), portfolio->getTotalReturnPercent(), 0.01);

    portfolio->buyStock(techCompany, 20, 100.0, 0.01, 1);
    ASSERT_TRUE(portfolio->hasPosition("TECH"));

    portfolio->sellStock(techCompany, 20, 110.0, 0.01, 2);
    ASSERT_FALSE(portfolio->hasPosition("TECH"));

    ASSERT_NEAR(portfolio->getSectorAllocationPercent(Sector::Technology), 0.0, 0.01);
}