#include <iostream>
#include <memory>
#include "core/Market.hpp"
#include "core/Player.hpp"
#include "services/NewsService.hpp"
#include "services/PriceService.hpp"
#include "ui/screens/MarketScreen.hpp"
#include "ui/screens/CompanyScreen.hpp"
#include "utils/Console.hpp"
#include "utils/Date.hpp"

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

        // Create price service
        auto priceService = std::make_shared<PriceService>(market);
        priceService->initialize();

        // Set the starting date (March 1, 2023)
        Date currentDate(1, 3, 2023);

        // Generate some generic market news
        for (int i = 0; i < 5; i++) {
            // Set current date in news service to ensure correct date for generated news
            newsService->setCurrentDate(currentDate);

            auto news = newsService->generateDailyNews(2);
            newsService->applyNewsEffects(news);
        }

        // Create some company-specific news for a few companies
        auto companies = market->getCompanies();
        if (!companies.empty()) {
            // Create reference date (March 1, 2023)
            Date referenceDate(1, 3, 2023);

            // Create dates for each news item by advancing from reference date
            Date techNewsDate = referenceDate;
            techNewsDate.advanceDays(5); // 5 days after start (March 6, 2023)

            Date innovaNewsDate = referenceDate;
            innovaNewsDate.advanceDays(7); // 7 days after start (March 8, 2023)

            Date energyNewsDate = referenceDate;
            energyNewsDate.advanceDays(10); // 10 days after start (March 11, 2023)

            // Create news for first company (usually TechCorp) using Date
            News techNews(
                NewsType::Corporate,
                "New Product Launch",
                "The company announces a revolutionary new product line that analysts predict will increase market share.",
                0.03, // positive impact
                techNewsDate, // using Date object now
                companies[0]
            );
            newsService->addCustomNews(techNews);

            // Create news for second company (usually InnovaTech) using Date
            if (companies.size() > 1) {
                News innovaNews(
                    NewsType::Corporate,
                    "Quarterly Results",
                    "The company reports quarterly results exceeding analyst expectations.",
                    0.025, // positive impact
                    innovaNewsDate, // using Date object now
                    companies[1]
                );
                newsService->addCustomNews(innovaNews);
            }

            // Create news for energy company using Date
            if (companies.size() > 2) {
                News energyNews(
                    NewsType::Corporate,
                    "Regulatory Challenge",
                    "The company faces new regulatory challenges that may impact future profits.",
                    -0.02, // negative impact
                    energyNewsDate, // using Date object now
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

            // Update current date for news service to match the market
            newsService->setCurrentDate(market->getCurrentDate());
        }

        // Create and configure market screen
        auto marketScreen = std::make_shared<MarketScreen>();
        marketScreen->setMarket(market);
        marketScreen->setPlayer(player);

        // Also pass the news service to the market screen
        // This allows the market to pass it to company screens when they're created
        marketScreen->setNewsService(newsService);

        // Position the screen
        marketScreen->setPosition(0, 0);

        // Initialize and run the market screen
        marketScreen->initialize();
        marketScreen->run();

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