#include <iostream>
#include <memory>
#include "core/Market.hpp"
#include "core/Player.hpp"
#include "services/NewsService.hpp"
#include "ui/screens/MainScreen.hpp"
#include "ui/screens/MarketScreen.hpp"
#include "ui/screens/CompanyScreen.hpp"
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

        // Create some company-specific news for a few companies
        auto companies = market->getCompanies();
        if (!companies.empty()) {
            // Create news for first company (usually TechCorp)
            News techNews(
                NewsType::Corporate,
                "New Product Launch",
                "The company announces a revolutionary new product line that analysts predict will increase market share.",
                0.03, // positive impact
                5,    // day 5
                companies[0]
            );
            newsService->addCustomNews(techNews);

            // Create news for second company (usually InnovaTech)
            if (companies.size() > 1) {
                News innovaNews(
                    NewsType::Corporate,
                    "Quarterly Results",
                    "The company reports quarterly results exceeding analyst expectations.",
                    0.025, // positive impact
                    7,     // day 7
                    companies[1]
                );
                newsService->addCustomNews(innovaNews);
            }

            // Create news for energy company
            if (companies.size() > 2) {
                News energyNews(
                    NewsType::Corporate,
                    "Regulatory Challenge",
                    "The company faces new regulatory challenges that may impact future profits.",
                    -0.02, // negative impact
                    10,    // day 10
                    companies[2]
                );
                newsService->addCustomNews(energyNews);
            }
        }

        // Simulate a few days to generate some price history
        for (int i = 0; i < 30; i++) {
            market->simulateDay();
            player->updateDailyState();
            player->closeDay();
        }

        // Create main screen instead of directly showing market screen
        auto mainScreen = std::make_shared<MainScreen>();
        mainScreen->setMarket(market);
        mainScreen->setPlayer(player);
        mainScreen->setNewsService(newsService);

        // Position the screen
        mainScreen->setPosition(0, 0);

        // Initialize and run the main screen
        mainScreen->initialize();
        mainScreen->run();

        // Clean up
        Console::clear();
        Console::setCursorPosition(0, 0);
        Console::setColor(TextColor::White, TextColor::Default);
        Console::println("Thank you for using Stock Player. Goodbye!");

        // Restore terminal settings
        Console::cleanup();

        return 0;
    }
    catch (const std::exception& e) {
        Console::clear();
        Console::setCursorPosition(0, 0);
        Console::setColor(TextColor::Red, TextColor::Default);
        Console::println("Error: " + std::string(e.what()));

        // Restore terminal settings even in case of error
        Console::cleanup();

        return 1;
    }
}