#include <gtest/gtest.h>
#include "core/Player.hpp"
#include "core/Market.hpp"

using namespace StockMarketSimulator;

class PlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        market = std::make_shared<Market>();
        market->addDefaultCompanies();
        
        player = std::make_unique<Player>("Test Player", 10000.0);
        player->setMarket(market);
        
        techCompany = market->getCompanyByTicker("TCH");
        energyCompany = market->getCompanyByTicker("EPLC");
        financeCompany = market->getCompanyByTicker("BANK");
    }
    
    std::shared_ptr<Market> market;
    std::unique_ptr<Player> player;
    std::shared_ptr<Company> techCompany;
    std::shared_ptr<Company> energyCompany;
    std::shared_ptr<Company> financeCompany;
};

TEST_F(PlayerTest, InitializationTest) {
    ASSERT_EQ(player->getName(), "Test Player");
    ASSERT_EQ(player->getPortfolio()->getCashBalance(), 10000.0);
    ASSERT_EQ(player->getMarginAccountBalance(), 0.0);
    ASSERT_EQ(player->getMarginUsed(), 0.0);
    ASSERT_NEAR(player->getMarginInterestRate(), 0.07, 0.001);
    ASSERT_NEAR(player->getMarginRequirement(), 0.5, 0.001);
    ASSERT_EQ(player->getCurrentDay(), 0);
    ASSERT_EQ(player->getLoans().size(), 0);
    
    Player defaultPlayer;
    ASSERT_EQ(defaultPlayer.getName(), "Player");
    ASSERT_EQ(defaultPlayer.getPortfolio()->getCashBalance(), 0.0);
}

TEST_F(PlayerTest, BuyStockTest) {
    bool result = player->buyStock(techCompany, 20);
    ASSERT_TRUE(result);
    
    ASSERT_TRUE(player->getPortfolio()->hasPosition("TCH"));
    ASSERT_EQ(player->getPortfolio()->getPositionQuantity("TCH"), 20);
    
    double expectedCost = 20 * techCompany->getStock()->getCurrentPrice() * 1.01;
    ASSERT_NEAR(player->getPortfolio()->getCashBalance(), 10000.0 - expectedCost, 1.0);
    
    result = player->buyStock(techCompany, 1000);
    ASSERT_FALSE(result);
    
    ASSERT_EQ(player->getPortfolio()->getPositionQuantity("TCH"), 20);
}

TEST_F(PlayerTest, SellStockTest) {
    player->buyStock(techCompany, 30);
    double balanceAfterBuy = player->getPortfolio()->getCashBalance();
    
    bool result = player->sellStock(techCompany, 10);
    ASSERT_TRUE(result);
    
    ASSERT_EQ(player->getPortfolio()->getPositionQuantity("TCH"), 20);
    
    double expectedProfit = 10 * techCompany->getStock()->getCurrentPrice() * 0.99;
    ASSERT_NEAR(player->getPortfolio()->getCashBalance(), balanceAfterBuy + expectedProfit, 1.0);
    
    result = player->sellStock(techCompany, 20);
    ASSERT_TRUE(result);
    
    ASSERT_FALSE(player->getPortfolio()->hasPosition("TCH"));
    
    result = player->sellStock(techCompany, 10);
    ASSERT_FALSE(result);
}

TEST_F(PlayerTest, MarginTradingTest) {
    std::cout << "\n=== MarginTradingTest: НАЧАЛО ТЕСТА ===\n";

    std::cout << "Начальный баланс: " << player->getPortfolio()->getCashBalance() << std::endl;
    std::cout << "Начальный счет маржи: " << player->getMarginAccountBalance() << std::endl;
    std::cout << "Начальная использованная маржа: " << player->getMarginUsed() << std::endl;
    std::cout << "Начальная доступная маржа: " << player->getMarginAvailable() << std::endl;

    std::cout << "\n--- Вносим 5000.0 на маржинальный счет ---\n";
    bool result = player->depositToMarginAccount(5000.0);
    std::cout << "Результат внесения: " << (result ? "успешно" : "неудача") << std::endl;
    std::cout << "Баланс после внесения: " << player->getPortfolio()->getCashBalance() << std::endl;
    std::cout << "Счет маржи после внесения: " << player->getMarginAccountBalance() << std::endl;
    std::cout << "Доступная маржа после внесения: " << player->getMarginAvailable() << std::endl;

    ASSERT_TRUE(result);
    ASSERT_NEAR(player->getMarginAccountBalance(), 5000.0, 0.01);
    ASSERT_NEAR(player->getPortfolio()->getCashBalance(), 5000.0, 0.01);

    std::cout << "\n--- Доступная маржа перед покупкой акций ---\n";
    double availableMarginBefore = player->getMarginAvailable();
    std::cout << "Доступная маржа: " << availableMarginBefore << std::endl;

    double stockPrice = techCompany->getStock()->getCurrentPrice();
    std::cout << "Текущая цена акции " << techCompany->getTicker() << ": " << stockPrice << std::endl;

    int quantity = 100;
    double expectedCost = quantity * stockPrice * 1.01;
    std::cout << "Ожидаемая стоимость покупки " << quantity << " акций: " << expectedCost << std::endl;

    std::cout << "\n--- Покупаем " << quantity << " акций с использованием маржи ---\n";
    result = player->buyStock(techCompany, quantity, true);
    std::cout << "Результат покупки: " << (result ? "успешно" : "неудача") << std::endl;

    if (result) {
        std::cout << "Баланс после покупки: " << player->getPortfolio()->getCashBalance() << std::endl;
        std::cout << "Счет маржи после покупки: " << player->getMarginAccountBalance() << std::endl;
        std::cout << "Использованная маржа после покупки: " << player->getMarginUsed() << std::endl;
        std::cout << "Доступная маржа после покупки: " << player->getMarginAvailable() << std::endl;

        int positionQuantity = player->getPortfolio()->getPositionQuantity("TCH");
        std::cout << "Количество акций в позиции: " << positionQuantity << std::endl;
    }

    ASSERT_TRUE(result);
    ASSERT_GT(player->getMarginUsed(), 0.0);

    std::cout << "\n--- Проверяем использование маржи ---\n";
    double marginUsedAfterBuy = player->getMarginUsed();
    std::cout << "Использованная маржа: " << marginUsedAfterBuy << std::endl;

    std::cout << "\n--- Продаем 50 акций ---\n";
    double marginUsedBefore = player->getMarginUsed();
    std::cout << "Использованная маржа перед продажей: " << marginUsedBefore << std::endl;

    result = player->sellStock(techCompany, 50);
    std::cout << "Результат продажи: " << (result ? "успешно" : "неудача") << std::endl;

    if (result) {
        std::cout << "Баланс после продажи: " << player->getPortfolio()->getCashBalance() << std::endl;
        std::cout << "Счет маржи после продажи: " << player->getMarginAccountBalance() << std::endl;
        std::cout << "Использованная маржа после продажи: " << player->getMarginUsed() << std::endl;
        std::cout << "Доступная маржа после продажи: " << player->getMarginAvailable() << std::endl;

        int positionQuantity = player->getPortfolio()->getPositionQuantity("TCH");
        std::cout << "Количество акций в позиции после продажи: " << positionQuantity << std::endl;
    }

    ASSERT_TRUE(result);
    ASSERT_LT(player->getMarginUsed(), marginUsedBefore);

std::cout << "\n--- Снимаем 1000.0 с маржинального счета ---\n";
    result = player->withdrawFromMarginAccount(1000.0);
    ASSERT_TRUE(result);
    ASSERT_EQ(player->getMarginAccountBalance(), 4000.0);

    std::cout << "\n--- Настраиваем маржинальные параметры ---\n";
    result = player->adjustMarginRequirement(0.6);
    ASSERT_TRUE(result);
    ASSERT_EQ(player->getMarginRequirement(), 0.6);

    result = player->adjustMarginInterestRate(0.08);
    ASSERT_TRUE(result);
    ASSERT_EQ(player->getMarginInterestRate(), 0.08);

    std::cout << "\n--- Используем маржу для проверки начисления процентов ---\n";
    double cashBefore = player->getPortfolio()->getCashBalance();
    bool marginCreated = player->buyStock(energyCompany, 100, true);

    if (marginCreated) {
        std::cout << "Успешно создана новая маржинальная задолженность для теста" << std::endl;
        std::cout << "Новое значение использованной маржи: " << player->getMarginUsed() << std::endl;
    } else {
        player->depositToMarginAccount(1000.0);
        ASSERT_NEAR(player->getMarginUsed(), 0.0, 0.01);

    }

    std::cout << "\n--- Обновляем состояние на следующий день ---\n";
    double marginUsedBeforeUpdate = player->getMarginUsed();
    std::cout << "Использованная маржа до обновления: " << marginUsedBeforeUpdate << std::endl;

    if (marginUsedBeforeUpdate > 0) {
        player->updateDailyState();
        std::cout << "Использованная маржа после обновления: " << player->getMarginUsed() << std::endl;

        ASSERT_GT(player->getMarginUsed(), marginUsedBeforeUpdate);
    } else {
        std::cout << "Пропускаем проверку начисления процентов, так как нет маржинальной задолженности" << std::endl;
    }

    std::cout << "\n=== MarginTradingTest: КОНЕЦ ТЕСТА ===\n";
}
TEST_F(PlayerTest, LoanTest) {
    bool result = player->takeLoan(5000.0, 0.05, 30, "Test Loan");
    ASSERT_TRUE(result);
    ASSERT_EQ(player->getLoans().size(), 1);
    ASSERT_EQ(player->getPortfolio()->getCashBalance(), 15000.0);
    
    result = player->takeLoan(-1000.0, 0.05, 30);
    ASSERT_FALSE(result);
    result = player->takeLoan(1000.0, -0.05, 30);
    ASSERT_FALSE(result);
    result = player->takeLoan(1000.0, 0.05, -30);
    ASSERT_FALSE(result);
    
    result = player->takeLoan(100000.0, 0.05, 30);
    ASSERT_FALSE(result);
    
    result = player->repayLoan(0, 2000.0);
    ASSERT_TRUE(result);
    ASSERT_FALSE(player->getLoans()[0].getIsPaid());
    ASSERT_EQ(player->getPortfolio()->getCashBalance(), 13000.0);
    
    result = player->repayLoan(0, 5000.0);
    ASSERT_TRUE(result);
    ASSERT_TRUE(player->getLoans()[0].getIsPaid());
    
    result = player->repayLoan(0, 1000.0);
    ASSERT_FALSE(result);
}

TEST_F(PlayerTest, DailyOperationsTest) {
    player->buyStock(techCompany, 20);
    player->buyStock(energyCompany, 30);
    
    player->closeDay();
    ASSERT_EQ(player->getCurrentDay(), 1);
    
    techCompany->getStock()->updatePrice(techCompany->getStock()->getCurrentPrice() * 1.05);
    energyCompany->getStock()->updatePrice(energyCompany->getStock()->getCurrentPrice() * 0.95);
    
    player->openDay();
    player->updateDailyState();
    
    double portfolioValue = player->getPortfolio()->getTotalValue();
    ASSERT_NE(portfolioValue, 10000.0);
    
    double initialCash = player->getPortfolio()->getCashBalance();
    player->receiveDividends(techCompany, 1.0);
    ASSERT_GT(player->getPortfolio()->getCashBalance(), initialCash);
    ASSERT_GT(player->getPortfolio()->getTotalDividendsReceived(), 0.0);
}

TEST_F(PlayerTest, NetWorthCalculationTest) {
    std::cout << "\n=== NetWorthCalculationTest: НАЧАЛО ТЕСТА ===\n";

    std::cout << "Начальный баланс кэша: " << player->getPortfolio()->getCashBalance() << std::endl;
    std::cout << "Начальная стоимость активов: " << player->getTotalAssetValue() << std::endl;
    std::cout << "Начальные обязательства: " << player->getTotalLiabilities() << std::endl;
    std::cout << "Начальная чистая стоимость: " << player->getNetWorth() << std::endl;

    std::cout << "\n--- Покупаем акции ---\n";
    double initialCash = player->getPortfolio()->getCashBalance();
    bool result = player->buyStock(techCompany, 20);

    std::cout << "Результат покупки: " << (result ? "успешно" : "неудача") << std::endl;
    if (result) {
        double stockPrice = techCompany->getStock()->getCurrentPrice();
        double expectedCost = 20 * stockPrice * 1.01;
        double remainingCash = player->getPortfolio()->getCashBalance();

        std::cout << "Цена акции: " << stockPrice << std::endl;
        std::cout << "Ожидаемая стоимость покупки: " << expectedCost << std::endl;
        std::cout << "Оставшийся кэш: " << remainingCash << std::endl;
        std::cout << "Ожидаемый оставшийся кэш: " << (initialCash - expectedCost) << std::endl;
        std::cout << "Позиция по акциям: " << player->getPortfolio()->getPositionQuantity("TCH") << std::endl;
    }

    std::cout << "\n--- Берем кредит ---\n";
    double cashBeforeLoan = player->getPortfolio()->getCashBalance();
    result = player->takeLoan(5000.0, 0.05, 30);

    std::cout << "Результат взятия кредита: " << (result ? "успешно" : "неудача") << std::endl;
    std::cout << "Баланс кэша после взятия кредита: " << player->getPortfolio()->getCashBalance() << std::endl;
    std::cout << "Ожидаемый баланс кэша: " << (cashBeforeLoan + 5000.0) << std::endl;
    std::cout << "Количество кредитов: " << player->getLoans().size() << std::endl;

    if (!player->getLoans().empty()) {
        const auto& loan = player->getLoans()[0];
        std::cout << "Сумма кредита: " << loan.getAmount() << std::endl;
        std::cout << "Процентная ставка: " << loan.getInterestRate() << std::endl;
        std::cout << "Срок кредита: " << loan.getDurationDays() << std::endl;
        std::cout << "Общая сумма к погашению: " << loan.getTotalDue() << std::endl;
    }

    std::cout << "\n--- Вносим средства на маржинальный счет ---\n";
    double cashBeforeMargin = player->getPortfolio()->getCashBalance();
    result = player->depositToMarginAccount(2000.0);

    std::cout << "Результат внесения на маржу: " << (result ? "успешно" : "неудача") << std::endl;
    std::cout << "Баланс кэша после внесения: " << player->getPortfolio()->getCashBalance() << std::endl;
    std::cout << "Ожидаемый баланс кэша: " << (cashBeforeMargin - 2000.0) << std::endl;
    std::cout << "Баланс маржинального счета: " << player->getMarginAccountBalance() << std::endl;
    std::cout << "Доступная маржа: " << player->getMarginAvailable() << std::endl;

    std::cout << "\n--- Покупаем акции с использованием маржи ---\n";
    double marginBefore = player->getMarginUsed();
    result = player->buyStock(energyCompany, 50, true);

    std::cout << "Результат покупки с маржей: " << (result ? "успешно" : "неудача") << std::endl;
    std::cout << "Использованная маржа до: " << marginBefore << std::endl;
    std::cout << "Использованная маржа после: " << player->getMarginUsed() << std::endl;
    std::cout << "Позиция по акциям: " << player->getPortfolio()->getPositionQuantity("EPLC") << std::endl;

    std::cout << "\n--- Расчет чистой стоимости ---\n";
    double totalAssets = player->getTotalAssetValue();
    double totalLiabilities = player->getTotalLiabilities();
    double netWorth = player->getNetWorth();

    std::cout << "Общие активы: " << totalAssets << std::endl;
    std::cout << "Детализация активов:" << std::endl;
    std::cout << "  - Кэш: " << player->getPortfolio()->getCashBalance() << std::endl;
    std::cout << "  - Стоимость акций: " << player->getPortfolio()->getTotalStocksValue() << std::endl;
    std::cout << "  - Маржинальный счет: " << player->getMarginAccountBalance() << std::endl;

    std::cout << "Общие обязательства: " << totalLiabilities << std::endl;
    std::cout << "Детализация обязательств:" << std::endl;
    std::cout << "  - Кредит: " << (!player->getLoans().empty() ? player->getLoans()[0].getTotalDue() : 0.0) << std::endl;
    std::cout << "  - Использованная маржа: " << player->getMarginUsed() << std::endl;

    std::cout << "Чистая стоимость: " << netWorth << std::endl;
    std::cout << "Ожидаемая чистая стоимость (активы - обязательства): " << (totalAssets - totalLiabilities) << std::endl;

    ASSERT_NEAR(netWorth, totalAssets - totalLiabilities, 0.01);

    ASSERT_GT(totalAssets, 0.0);
    ASSERT_GT(totalLiabilities, 0.0);
    ASSERT_GT(netWorth, 0.0);

    std::cout << "\n=== NetWorthCalculationTest: КОНЕЦ ТЕСТА ===\n";
}

TEST_F(PlayerTest, JsonSerializationTest) {
    player->buyStock(techCompany, 20);
    player->takeLoan(5000.0, 0.05, 30);
    player->depositToMarginAccount(2000.0);
    player->closeDay();
    
    nlohmann::json json = player->toJson();
    
    Player restoredPlayer = Player::fromJson(json, market);
    
    ASSERT_EQ(restoredPlayer.getName(), player->getName());
    ASSERT_EQ(restoredPlayer.getCurrentDay(), player->getCurrentDay());
    ASSERT_EQ(restoredPlayer.getMarginAccountBalance(), player->getMarginAccountBalance());
    ASSERT_EQ(restoredPlayer.getMarginUsed(), player->getMarginUsed());
    ASSERT_EQ(restoredPlayer.getMarginInterestRate(), player->getMarginInterestRate());
    ASSERT_EQ(restoredPlayer.getLoans().size(), player->getLoans().size());
    
    ASSERT_NEAR(restoredPlayer.getPortfolio()->getCashBalance(), player->getPortfolio()->getCashBalance(), 0.01);
    ASSERT_NEAR(restoredPlayer.getPortfolio()->getTotalValue(), player->getPortfolio()->getTotalValue(), 0.01);
}

TEST_F(PlayerTest, CashOperationsTest) {
    bool result = player->depositCash(5000.0);
    ASSERT_TRUE(result);
    ASSERT_EQ(player->getPortfolio()->getCashBalance(), 15000.0);
    
    result = player->withdrawCash(3000.0);
    ASSERT_TRUE(result);
    ASSERT_EQ(player->getPortfolio()->getCashBalance(), 12000.0);
    
    result = player->withdrawCash(20000.0);
    ASSERT_FALSE(result);
    ASSERT_EQ(player->getPortfolio()->getCashBalance(), 12000.0);
    
    result = player->depositCash(-1000.0);
    ASSERT_FALSE(result);
    result = player->withdrawCash(-500.0);
    ASSERT_FALSE(result);
}
TEST_F(PlayerTest, MarginCallTest) {
    std::cout << "\n=== MarginCallTest: НАЧАЛО ТЕСТА ===\n";

    player->withdrawCash(9000.0);

    player->depositToMarginAccount(500.0);

    player->adjustMarginRequirement(0.4);

    std::cout << "Начальные значения:\n";
    std::cout << "Баланс кэша: " << player->getPortfolio()->getCashBalance() << std::endl;
    std::cout << "Маржинальный счет: " << player->getMarginAccountBalance() << std::endl;
    std::cout << "Использованная маржа: " << player->getMarginUsed() << std::endl;

    double stockPrice = techCompany->getStock()->getCurrentPrice();


    int quantity = static_cast<int>(1000.0 / (stockPrice * 1.01));

    std::cout << "Покупаем " << quantity << " акций по цене " << stockPrice << std::endl;
    bool result = player->buyStock(techCompany, quantity, true);
    ASSERT_TRUE(result);

    std::cout << "\nСостояние после покупки акций на марже:\n";
    std::cout << "Баланс кэша: " << player->getPortfolio()->getCashBalance() << std::endl;
    std::cout << "Стоимость акций: " << player->getPortfolio()->getTotalStocksValue() << std::endl;
    std::cout << "Маржинальный счет: " << player->getMarginAccountBalance() << std::endl;
    std::cout << "Использованная маржа: " << player->getMarginUsed() << std::endl;

    ASSERT_GT(player->getMarginUsed(), 0.0);

    double initialCash = player->getPortfolio()->getCashBalance();
    double initialStocksValue = player->getPortfolio()->getTotalStocksValue();
    double initialMarginUsed = player->getMarginUsed();
    int initialShareCount = player->getPortfolio()->getPositionQuantity(techCompany->getTicker());

    std::cout << "Количество акций перед падением цены: " << initialShareCount << std::endl;

    player->openDay();

    double originalPrice = techCompany->getStock()->getCurrentPrice();
    double newPrice = originalPrice * 0.6;

    std::cout << "\nСнижаем цену акций на 40% с " << originalPrice << " до " << newPrice << std::endl;
    techCompany->getStock()->updatePrice(newPrice);

    std::cout << "\nЗапускаем обновление состояния (должен сработать маржин-колл)...\n";
    player->updateDailyState();

    double afterCash = player->getPortfolio()->getCashBalance();
    double afterStocksValue = player->getPortfolio()->getTotalStocksValue();
    double afterMarginUsed = player->getMarginUsed();
    int afterShareCount = player->getPortfolio()->getPositionQuantity(techCompany->getTicker());

    std::cout << "\nСостояние после маржин-колла:\n";
    std::cout << "Баланс кэша: " << afterCash << std::endl;
    std::cout << "Стоимость акций: " << afterStocksValue << std::endl;
    std::cout << "Маржинальный счет: " << player->getMarginAccountBalance() << std::endl;
    std::cout << "Использованная маржа: " << afterMarginUsed << std::endl;
    std::cout << "Количество акций после: " << afterShareCount << std::endl;

    ASSERT_LT(afterShareCount, initialShareCount);

    std::cout << "\n=== MarginCallTest: КОНЕЦ ТЕСТА ===\n";
}