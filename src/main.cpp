#include <memory>
#include <iostream>
#include "utils/Console.hpp"
#include "core/Market.hpp"
#include "core/Player.hpp"
#include "services/NewsService.hpp"
#include "ui/screens/NewsScreen.hpp"

using namespace StockMarketSimulator;

int main() {
    try {
        // Initialize console
        Console::initialize();
        Console::clear();

        // Set up market
        auto market = std::make_shared<Market>();
        market->addDefaultCompanies();

        // Set up initial game day
        for (int i = 0; i < 15; i++) {
            market->simulateDay();
        }

        // Set up player
        auto player = std::make_shared<Player>("Test Player", 10000.0);
        player->setMarket(market);

        // Set up news service
        auto newsService = std::make_shared<NewsService>(market);

        // Add sample news
        // 1. Global news
        News globalNews1(NewsType::Global, "Central Bank reduced rate to 3.5%",
                      "The Central Bank announced an unexpected reduction in the base interest rate from 4.0% to 3.5%. "
                      "Analysts believe this move aims to stimulate economic growth amid concerns of a potential slowdown. "
                      "Markets responded positively to the news, with most indices showing gains.",
                      0.02, 15);
        newsService->addCustomNews(globalNews1);

        News globalNews2(NewsType::Global, "Inflation data published: 2.1%",
                      "The National Bureau of Statistics released the latest inflation figures, showing an annual inflation rate of 2.1%. "
                      "This is slightly below market expectations of 2.3%, indicating that inflationary pressures may be easing. "
                      "This could influence future monetary policy decisions.",
                      0.01, 13);
        newsService->addCustomNews(globalNews2);

        // 2. Sector news
        News energySectorNews(NewsType::Sector, "Oil prices rose by 2.5%",
                           "Global oil prices increased by 2.5% following reports of reduced output from major producing countries. "
                           "The price surge is expected to impact profit margins across the energy sector, with downstream companies "
                           "potentially facing increased costs.",
                           -0.01, 14, Sector::Energy);
        newsService->addCustomNews(energySectorNews);

        News techSectorNews(NewsType::Sector, "Tech sector sees workforce reductions",
                         "Major technology companies announced plans to reduce their workforce by an average of 5% due to economic uncertainty. "
                         "The job cuts are expected to help companies maintain profitability amid slowing revenue growth.",
                         -0.015, 12, Sector::Technology);
        newsService->addCustomNews(techSectorNews);

        // 3. Company news
        auto techCorp = market->getCompanyByTicker("TCH");
        News companyNews1(NewsType::Corporate, "TechCorp announced a new product",
                       "TechCorp unveiled its latest flagship product during its annual developer conference. "
                       "The new device features significant performance improvements and several innovative features. "
                       "Pre-orders have exceeded initial projections by 35%.",
                       0.03, 14, techCorp);
        newsService->addCustomNews(companyNews1);

        auto bankCo = market->getCompanyByTicker("BANK");
        News companyNews2(NewsType::Corporate, "BankCo announced dividend payments",
                       "BankCo's board of directors approved a quarterly dividend payment of $0.45 per share, "
                       "representing a 5% increase from the previous quarter. The dividend will be paid "
                       "to shareholders of record as of the end of the month.",
                       0.015, 12, bankCo);
        newsService->addCustomNews(companyNews2);

        // Create and display the NewsScreen
        auto newsScreen = std::make_shared<NewsScreen>();
        newsScreen->setMarket(market);
        newsScreen->setPlayer(player);
        newsScreen->setNewsService(newsService);

        // Initialize and run the screen
        newsScreen->initialize();
        newsScreen->run();

        // Clean up
        Console::clear();
        Console::setCursorPosition(0, 0);
        Console::resetAttributes();

        return 0;
    }
    catch (const std::exception& e) {
        Console::resetAttributes();
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}