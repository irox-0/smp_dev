#include <memory>
#include <iostream>
#include "core/Market.hpp"
#include "core/Player.hpp"
#include "services/NewsService.hpp"
#include "ui/screens/NewsScreen.hpp"
#include "utils/Console.hpp"

using namespace StockMarketSimulator;

// Demo program to showcase the NewsScreen
int main() {
    // Initialize console
    Console::initialize();
    Console::clear();

    std::cout << "Initializing Stock Market Simulator..." << std::endl;

    // Create core components
    auto market = std::make_shared<Market>();
    auto player = std::make_shared<Player>("Investor", 10000.0);
    auto newsService = std::make_shared<NewsService>(market);

    // Setup market
    market->addDefaultCompanies();
    player->setMarket(market);

    // Set current day
    for (int i = 0; i < 15; i++) {
        market->simulateDay();
    }

    std::cout << "Generating news items..." << std::endl;

    // Create sample news items
    auto techCorp = market->getCompanyByTicker("TCH");
    auto bankCo = market->getCompanyByTicker("BANK");
    auto energyPlus = market->getCompanyByTicker("EPLC");

    // Global news
    News globalNews1(NewsType::Global, "Central Bank reduced rate to 3.5%",
                    "The Central Bank announced today that it has reduced the key interest rate by 0.5 percentage points to 3.5%. This decision was made to stimulate economic growth amid signs of slowing inflation. Analysts expect this move to have a positive impact on stock markets, particularly in sectors sensitive to interest rates such as Finance and Technology.",
                    0.02, 15);
    newsService->addCustomNews(globalNews1);

    // Company news
    News companyNews1(NewsType::Corporate, "TechCorp announced a new product",
                     "TechCorp today unveiled its latest flagship product at a press conference in Silicon Valley. The new device features cutting-edge technology that the company claims will 'revolutionize the industry'. Market analysts predict strong sales, potentially boosting the company's revenue by up to 15% in the next quarter.",
                     0.035, 15, techCorp);
    newsService->addCustomNews(companyNews1);

    // Sector news
    News sectorNews1(NewsType::Sector, "Oil prices rose by 2.5%",
                    "Global oil prices increased by 2.5% today following reports of reduced production in major oil-producing countries. This price surge is expected to benefit companies in the Energy sector, while potentially increasing costs for Transportation and Manufacturing businesses that rely heavily on fuel.",
                    0.015, 14, Sector::Energy);
    newsService->addCustomNews(sectorNews1);

    // More news
    News globalNews2(NewsType::Global, "Inflation data published: 2.1%",
                    "Government statistics released today show that inflation for the last month stood at 2.1%, slightly below the 2.3% forecast by economists. This better-than-expected figure suggests that the economy is stabilizing, which could delay further interest rate hikes by the Central Bank.",
                    -0.01, 13);
    newsService->addCustomNews(globalNews2);

    News companyNews2(NewsType::Corporate, "BankCo announced dividend payments",
                     "BankCo's board of directors has approved a quarterly dividend of $0.45 per share, representing a 5% increase from the previous quarter. The dividend will be payable to shareholders of record on the 25th of this month. The company cited strong financial performance and a positive outlook as reasons for the dividend increase.",
                     0.02, 12, bankCo);
    newsService->addCustomNews(companyNews2);

    News sectorNews2(NewsType::Sector, "Tech sector reports strong growth",
                    "The technology sector has reported an average revenue growth of 8.3% in the last quarter, outperforming market expectations of 6.5%. This strong performance is attributed to increased consumer spending on electronics and software, as well as growing enterprise investments in cloud services and digital transformation initiatives.",
                    0.025, 11, Sector::Technology);
    newsService->addCustomNews(sectorNews2);

    News companyNews3(NewsType::Corporate, "EnergyPlus expands renewable portfolio",
                     "EnergyPlus has announced the acquisition of three wind farms, significantly expanding its renewable energy portfolio. The company expects these assets to contribute approximately 12% to its annual electricity generation capacity and to be accretive to earnings within the first year of operation.",
                     0.018, 10, energyPlus);
    newsService->addCustomNews(companyNews3);

    // Create and configure NewsScreen
    std::cout << "Launching News Screen..." << std::endl;
    auto newsScreen = std::make_shared<NewsScreen>();
    newsScreen->setPosition(0, 0);
    newsScreen->setSize(50, 24);
    newsScreen->setMarket(market);
    newsScreen->setPlayer(player);
    newsScreen->setNewsService(newsService);

    // Initialize and run the screen
    newsScreen->initialize();
    Console::clear();
    newsScreen->run();

    // Clean up
    Console::clear();
    Console::setCursorPosition(0, 0);
    std::cout << "Thank you for using the Stock Market Simulator!" << std::endl;

    return 0;
}