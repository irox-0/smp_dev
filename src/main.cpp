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
        Console::initialize();
        Console::clear();

        auto market = std::make_shared<Market>();
        market->addDefaultCompanies();

        auto player = std::make_shared<Player>("Trader", 10000.0);
        player->setMarket(market);

        auto newsService = std::make_shared<NewsService>(market);
        newsService->initialize();

        for (int i = 0; i < 5; i++) {
            auto news = newsService->generateDailyNews(2);
            newsService->applyNewsEffects(news);
        }

        auto companies = market->getCompanies();
        if (!companies.empty()) {
            News techNews(
                NewsType::Corporate,
                "New Product Launch",
                "The company announces a revolutionary new product line that analysts predict will increase market share.",
                0.03,
                5,
                companies[0]
            );
            newsService->addCustomNews(techNews);

            if (companies.size() > 1) {
                News innovaNews(
                    NewsType::Corporate,
                    "Quarterly Results",
                    "The company reports quarterly results exceeding analyst expectations.",
                    0.025,
                    7,
                    companies[1]
                );
                newsService->addCustomNews(innovaNews);
            }

            if (companies.size() > 2) {
                News energyNews(
                    NewsType::Corporate,
                    "Regulatory Challenge",
                    "The company faces new regulatory challenges that may impact future profits.",
                    -0.02,
                    10,
                    companies[2]
                );
                newsService->addCustomNews(energyNews);
            }
        }

        for (int i = 0; i < 30; i++) {
            market->simulateDay();
            player->updateDailyState();
            player->closeDay();
        }

        auto mainScreen = std::make_shared<MainScreen>();
        mainScreen->setMarket(market);
        mainScreen->setPlayer(player);
        mainScreen->setNewsService(newsService);

        mainScreen->setPosition(0, 0);

        mainScreen->initialize();
        mainScreen->run();

        Console::clear();
        Console::setCursorPosition(0, 0);
        Console::setColor(TextColor::White, TextColor::Default);
        Console::println("Thank you for using Stock Player. Goodbye!");

        Console::cleanup();

        return 0;
    }
    catch (const std::exception& e) {
        Console::clear();
        Console::setCursorPosition(0, 0);
        Console::setColor(TextColor::Red, TextColor::Default);
        Console::println("Error: " + std::string(e.what()));

        Console::cleanup();

        return 1;
    }
}