#include <gtest/gtest.h>
#include <memory>
#include "../../../src/core/Market.hpp"
#include "../../../src/core/Player.hpp"
#include "../../../src/ui/screens/FinancialScreen.hpp"

using namespace StockMarketSimulator;

class FinancialScreenTest : public ::testing::Test {
protected:
    std::shared_ptr<Market> market;
    std::shared_ptr<Player> player;
    std::shared_ptr<FinancialScreen> screen;

    void SetUp() override {
        market = std::make_shared<Market>();
        market->addDefaultCompanies();

        player = std::make_shared<Player>("Test Player", 10000.0);
        player->setMarket(market);

        screen = std::make_shared<FinancialScreen>();
        screen->setMarket(market);
        screen->setPlayer(player);
        screen->initialize();
    }
};

TEST_F(FinancialScreenTest, Initialization) {
    EXPECT_EQ(screen->getTitle(), "FINANCIAL INSTRUMENTS");
    EXPECT_EQ(screen->getType(), ScreenType::Financial);
    EXPECT_EQ(screen->getCurrentSection(), FinancialSection::Loans);
    EXPECT_EQ(screen->getSelectedLoanIndex(), -1);
}

TEST_F(FinancialScreenTest, SectionToggle) {
    EXPECT_EQ(screen->getCurrentSection(), FinancialSection::Loans);

    screen->setCurrentSection(FinancialSection::MarginAccount);
    EXPECT_EQ(screen->getCurrentSection(), FinancialSection::MarginAccount);

    screen->setCurrentSection(FinancialSection::Loans);
    EXPECT_EQ(screen->getCurrentSection(), FinancialSection::Loans);
}

TEST_F(FinancialScreenTest, HandleLoanOperations) {
    double initialBalance = player->getPortfolio()->getCashBalance();
    double loanAmount = 5000.0;

    screen->processTakeLoan(loanAmount, 0.05, 30, "Test Loan");

    EXPECT_EQ(player->getLoans().size(), 1);
    EXPECT_NEAR(player->getPortfolio()->getCashBalance(), initialBalance + loanAmount, 0.01);

    screen->processLoanRepayment(0);

    EXPECT_TRUE(player->getLoans()[0].getIsPaid());
    EXPECT_NEAR(player->getPortfolio()->getCashBalance(), initialBalance, 0.1);
}

TEST_F(FinancialScreenTest, HandleMarginOperations) {
    double initialBalance = player->getPortfolio()->getCashBalance();
    double depositAmount = 2000.0;

    bool depositResult = player->depositToMarginAccount(depositAmount);
    EXPECT_TRUE(depositResult);
    EXPECT_NEAR(player->getMarginAccountBalance(), depositAmount, 0.01);
    EXPECT_NEAR(player->getPortfolio()->getCashBalance(), initialBalance - depositAmount, 0.01);

    bool withdrawResult = player->withdrawFromMarginAccount(depositAmount);
    EXPECT_TRUE(withdrawResult);
    EXPECT_NEAR(player->getMarginAccountBalance(), 0.0, 0.01);
    EXPECT_NEAR(player->getPortfolio()->getCashBalance(), initialBalance, 0.01);
}

TEST_F(FinancialScreenTest, InputHandling) {
    EXPECT_TRUE(screen->handleInput('1'));
    EXPECT_TRUE(screen->handleInput('2'));
    EXPECT_TRUE(screen->handleInput('3'));
    EXPECT_FALSE(screen->handleInput('0'));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
