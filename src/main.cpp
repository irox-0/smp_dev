#include <iostream>
#include <memory>
#include "core/Market.hpp"
#include "core/Player.hpp"
#include "services/NewsService.hpp"
#include "ui/screens/MainScreen.hpp"
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

        // Generate some initial news
        for (int i = 0; i < 5; i++) {
            auto news = newsService->generateDailyNews(2);
            newsService->applyNewsEffects(news);
        }

        // Create company-specific news
        auto companies = market->getCompanies();
        if (!companies.empty()) {
            // Create news for TechCorp (first company)
            News techNews(
                NewsType::Corporate,
                "TechCorp announced a new product",
                "The company announced a groundbreaking new product that could revolutionize the market.",
                0.03, // positive impact
                5,    // day 5
                companies[0]
            );
            newsService->addCustomNews(techNews);

            // Create global economic news
            News economicNews(
                NewsType::Global,
                "Central Bank reduced rate to 3.5%",
                "The Central Bank has reduced the base interest rate to 3.5%, which may stimulate economic growth.",
                0.02, // positive impact
                7,    // day 7
                Sector::Finance
            );
            newsService->addCustomNews(economicNews);
        }

        // Simulate a few days to generate some price history
        for (int i = 0; i < 30; i++) {
            market->simulateDay();
            player->updateDailyState();
            player->closeDay();
        }

        // Create and configure main screen
        auto mainScreen = std::make_shared<MainScreen>();
        mainScreen->setMarket(market);
        mainScreen->setPlayer(player);
        mainScreen->setNewsService(newsService);

        // Center the screen (optional)
        auto consoleSize = Console::getSize();
        // int screenX = (consoleSize.first - mainScreen->getWidth()) / 2;
        // int screenY = (consoleSize.second - mainScreen->getHeight()) / 2;
        // mainScreen->setPosition(screenX, screenY);

        // Initialize and run the main screen
        mainScreen->initialize();
        mainScreen->run();

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