#include <iostream>
#include <memory>
#include "core/Market.hpp"
#include "core/Player.hpp"
#include "models/Company.hpp"
#include "ui/screens/FinancialScreen.hpp"
#include "utils/Console.hpp"

using namespace StockMarketSimulator;

int main() {
    Console::initialize();
    Console::clear();

    auto market = std::make_shared<Market>();
    market->addDefaultCompanies();

    auto player = std::make_shared<Player>("Test Player", 10000.0);
    player->setMarket(market);

    player->takeLoan(5000.0, 0.05, 30, "Short-term Loan");
    player->takeLoan(8000.0, 0.07, 90, "Medium-term Loan");

    auto financialScreen = std::make_shared<FinancialScreen>();
    financialScreen->setMarket(market);
    financialScreen->setPlayer(player);

    financialScreen->initialize();
    financialScreen->setPosition(0, 0);

    financialScreen->run();

    Console::clear();
    Console::setCursorPosition(0, 0);
    Console::println("Financial Screen Test Complete", TextColor::Green);

    return 0;
}