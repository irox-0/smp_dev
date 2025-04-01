#include <iostream>
#include <memory>
#include "core/Market.hpp"
#include "core/Player.hpp"
#include "services/NewsService.hpp"
#include "ui/screens/PortfolioScreen.hpp"
#include "utils/Console.hpp"

using namespace StockMarketSimulator;

int main() {
    try {
        // Initialize console
        Console::initialize();
        Console::clear();

        // Create market and add default companies
        auto market = std::make_shared<Market>();
        market->addDefaultCompanies();

        // Create player
        auto player = std::make_shared<Player>("Trader", 10000.0);
        player->setMarket(market);

        // Create news service
        auto newsService = std::make_shared<NewsService>(market);
        newsService->initialize();

        // Generate some generic market news
        for (int i = 0; i < 5; i++) {
            auto news = newsService->generateDailyNews(2);
            newsService->applyNewsEffects(news);
        }

        // Simulate some days to generate data
        for (int i = 0; i < 30; i++) {
            market->simulateDay();
            player->updateDailyState();
            player->closeDay();
        }

        // Buy some stocks to populate the portfolio
        auto techCorp = market->getCompanyByTicker("TCH");
        auto energyPlus = market->getCompanyByTicker("EPLC");
        auto bankCo = market->getCompanyByTicker("BANK");
        auto oilMax = market->getCompanyByTicker("OMAX");

        if (techCorp) player->buyStock(techCorp, 50);
        if (energyPlus) player->buyStock(energyPlus, 70);
        if (bankCo) player->buyStock(bankCo, 30);
        if (oilMax) player->buyStock(oilMax, 20);

        // Simulate a few more days to allow stocks to change in price
        for (int i = 0; i < 5; i++) {
            market->simulateDay();
            player->updateDailyState();
            player->closeDay();
        }

        // Create and configure portfolio screen
        auto portfolioScreen = std::make_shared<PortfolioScreen>();
        portfolioScreen->setMarket(market);
        portfolioScreen->setPlayer(player);

        // Position the screen
        portfolioScreen->setPosition(0, 0);

        // Initialize and run the portfolio screen
        portfolioScreen->initialize();
        portfolioScreen->run();

        // Clean up
        Console::clear();
        Console::setCursorPosition(0, 0);
        Console::setColor(TextColor::White, TextColor::Default);
        Console::println("Thank you for using Stock Player. Goodbye!");

        return 0;
    }
    catch (const std::exception& e) {
        Console::clear();
        Console::setCursorPosition(0, 0);
        Console::setColor(TextColor::Red, TextColor::Default);
        Console::println("Error: " + std::string(e.what()));
        return 1;
    }
}