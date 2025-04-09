#include "MainScreen.hpp"
#include "utils/FileIO.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <fstream>

namespace StockMarketSimulator {

MainScreen::MainScreen()
    : Screen("STOCK PLAYER - MAIN MENU", ScreenType::Main),
      game(nullptr)
{
    setSize(48, 32);
}

void MainScreen::setGame(std::shared_ptr<Game> game) {
    this->game = game;

    if (game) {
        setMarket(game->getMarket());
        setPlayer(game->getPlayer());
        setNewsService(game->getNewsService());
    }
}

std::shared_ptr<Game> MainScreen::getGame() const {
    return game;
}

void MainScreen::setNewsService(std::weak_ptr<NewsService> newsService) {
    this->newsService = newsService;
}

std::weak_ptr<NewsService> MainScreen::getNewsService() const {
    return newsService;
}

void MainScreen::initialize() {
    Screen::initialize();
    update();
}

void MainScreen::update() {
    Screen::update();
    updateTopStocks();
    updateLatestNews();

    checkGameOverConditions();
}

void MainScreen::checkGameOverConditions() {
    if (!game) return;

    auto playerPtr = player.lock();
    if (!playerPtr) return;

    if (playerPtr->getPortfolio()->getTotalValue() <= 0.01) {
        gameOver("Game Over: You've run out of money!");
        return;
    }

    if (playerPtr->getMarginLoan() > 0) {
        if (playerPtr->checkMarginCall()) {
            gameOver("Game Over: Margin Call! Your assets are worth less than your margin loan.");
            return;
        }
    }

    for (const auto& loan : playerPtr->getLoans()) {
        if (!loan.getIsPaid() && loan.isOverdue(playerPtr->getCurrentDate())) {
            if (loan.getTotalDue() > playerPtr->getPortfolio()->getCashBalance()) {
                gameOver("Game Over: Loan Default! You failed to repay a loan on time.");
                return;
            }
        }
    }
}

void MainScreen::gameOver(const std::string& message) {
    if (game) {
        game->pause();
    }

    int messageY = y + height / 2;

    for (int i = -2; i <= 2; i++) {
        Console::setCursorPosition(x + 2, messageY + i);
        Console::setColor(bodyFg, bodyBg);
        Console::print(std::string(width - 4, ' '));
    }

    Console::setCursorPosition(x + width/2 - 4, messageY - 1);
    Console::setColor(TextColor::Red, bodyBg);
    Console::print("GAME OVER");

    Console::setCursorPosition(x + width/2 - message.length()/2, messageY);
    Console::print(message);

    Console::setCursorPosition(x + width/2 - 13, messageY + 2);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Press any key to exit...");

    Console::readChar();
    close();
}

void MainScreen::updateTopStocks() {
    auto marketPtr = market.lock();
    if (!marketPtr) {
        topStocks.clear();
        return;
    }

    const auto& companies = marketPtr->getCompanies();
    if (companies.empty()) {
        topStocks.clear();
        return;
    }

    topStocks = companies;
    std::sort(topStocks.begin(), topStocks.end(),
              [](const auto& a, const auto& b) {
                  double changeA = a->getStock()->getDayChangePercent();
                  double changeB = b->getStock()->getDayChangePercent();
                  return changeA > changeB;
              });

    if (topStocks.size() > 5) {
        topStocks.resize(5);
    }
}

void MainScreen::updateLatestNews() {
    auto newsServicePtr = newsService.lock();
    if (!newsServicePtr) {
        latestNews.clear();
        return;
    }

    latestNews = newsServicePtr->getLatestNews(2);
}

void MainScreen::drawContent() const {
    drawDateAndMarket();
    drawPlayerInfo();
    drawTopStocks();
    drawLatestNews();
    drawNavigationMenu();
}

void MainScreen::drawDateAndMarket() const {
    auto marketPtr = market.lock();
    if (!marketPtr) {
        return;
    }

    int currentY = y + 2;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Date: ");
    Console::setColor(TextColor::Yellow, bodyBg);

    Date currentDate = marketPtr->getCurrentDate();
    int currentDay = marketPtr->getCurrentDay();

    std::stringstream dateStr;
    dateStr << currentDate.getDay() << "." << std::setfill('0') << std::setw(2) << currentDate.getMonth() << "." << currentDate.getYear();
    dateStr << " Day: " << currentDay;

    Console::print(dateStr.str());
    currentY += 1;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Market Index: ");

    double indexValue = marketPtr->getMarketIndex();
    double dailyChange = marketPtr->getState().dailyChange;

    std::stringstream indexStr;
    indexStr << std::fixed << std::setprecision(2) << indexValue;
    Console::setColor(TextColor::White, bodyBg);
    Console::print(indexStr.str());

    if (dailyChange >= 0) {
        Console::setColor(TextColor::Green, bodyBg);
        indexStr.str("");
        indexStr << " (+" << std::fixed << std::setprecision(2) << dailyChange << ")";
    } else {
        Console::setColor(TextColor::Red, bodyBg);
        indexStr.str("");
        indexStr << " (" << std::fixed << std::setprecision(2) << dailyChange << ")";
    }
    Console::print(indexStr.str());
    currentY += 1;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Trend: ");

    MarketTrend trend = marketPtr->getCurrentTrend();
    std::string trendName = marketPtr->getTrendName();

    if (trend == MarketTrend::Bullish) {
        Console::setColor(TextColor::Green, bodyBg);
    } else if (trend == MarketTrend::Bearish) {
        Console::setColor(TextColor::Red, bodyBg);
    } else if (trend == MarketTrend::Volatile) {
        Console::setColor(TextColor::Yellow, bodyBg);
    } else {
        Console::setColor(TextColor::Cyan, bodyBg);
    }
    Console::print(trendName);

    Console::setCursorPosition(x, y + 5);
    Console::setColor(bodyFg, bodyBg);
    Console::drawHorizontalLine(x, y + 5, width);
}

void MainScreen::drawPlayerInfo() const {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    int currentY = y + 6;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Your Capital: ");
    Console::setColor(TextColor::Green, bodyBg);

    double cashBalance = playerPtr->getPortfolio()->getCashBalance();
    std::stringstream cashStr;
    cashStr << std::fixed << std::setprecision(2) << cashBalance << "$";
    Console::print(cashStr.str());
    currentY += 1;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Portfolio Value: ");
    Console::setColor(TextColor::Cyan, bodyBg);

    double portfolioValue = playerPtr->getPortfolio()->getTotalStocksValue();
    std::stringstream portfolioStr;
    portfolioStr << std::fixed << std::setprecision(2) << portfolioValue << "$";
    Console::print(portfolioStr.str());
    currentY += 1;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Total Asset Value: ");
    Console::setColor(TextColor::Yellow, bodyBg);

    double totalAssets = playerPtr->getTotalAssetValue();
    std::stringstream assetsStr;
    assetsStr << std::fixed << std::setprecision(2) << totalAssets << "$";
    Console::print(assetsStr.str());

    Console::setCursorPosition(x, y + 9);
    Console::setColor(bodyFg, bodyBg);
    Console::drawHorizontalLine(x, y + 9, width);
}

void MainScreen::drawTopStocks() const {
    Console::setCursorPosition(x + 2, y + 10);
    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);
    Console::print("TOP STOCKS TODAY:");
    Console::setStyle(TextStyle::Regular);

    if (topStocks.empty()) {
        Console::setCursorPosition(x + 2, y + 12);
        Console::setColor(bodyFg, bodyBg);
        Console::print("No stock data available.");
        return;
    }

    int currentY = y + 11;
    for (size_t i = 0; i < topStocks.size(); i++) {
        const auto& company = topStocks[i];
        double changePercent = company->getStock()->getDayChangePercent();

        currentY += 1;
        Console::setCursorPosition(x + 2, currentY);
        Console::setColor(bodyFg, bodyBg);
        Console::print(std::to_string(i + 1) + ". " + company->getName() + " ");

        std::stringstream changeStr;
        if (changePercent >= 0) {
            Console::setColor(TextColor::Green, bodyBg);
            changeStr << "+" << std::fixed << std::setprecision(2) << changePercent << "%";
        } else {
            Console::setColor(TextColor::Red, bodyBg);
            changeStr << std::fixed << std::setprecision(2) << changePercent << "%";
        }
        Console::print(changeStr.str());
    }

    int separatorY = y + 11 + topStocks.size() + 1;
    Console::setCursorPosition(x, separatorY);
    Console::setColor(bodyFg, bodyBg);
    Console::drawHorizontalLine(x, separatorY, width);
}

void MainScreen::drawLatestNews() const {
    int newsY = y + 18;

    Console::setCursorPosition(x + 2, newsY);
    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);
    Console::print("LATEST NEWS:");
    Console::setStyle(TextStyle::Regular);
    newsY += 1;

    if (latestNews.empty()) {
        newsY += 1;
        Console::setCursorPosition(x + 2, newsY);
        Console::setColor(bodyFg, bodyBg);
        Console::print("No news available.");
        return;
    }

    for (const auto& news : latestNews) {
        newsY += 1;
        Console::setCursorPosition(x + 2, newsY);
        Console::setColor(TextColor::Cyan, bodyBg);
        Console::print("- " + news.getTitle());
    }

    int separatorY = newsY + 2;
    Console::setCursorPosition(x, separatorY);
    Console::setColor(bodyFg, bodyBg);
    Console::drawHorizontalLine(x, separatorY, width);
}

void MainScreen::drawNavigationMenu() const {
    int menuY = y + 23;

    Console::setCursorPosition(x + 2, menuY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("");
    menuY += 1;

    Console::setCursorPosition(x + 2, menuY);
    Console::print("1. Stock Market");
    menuY += 1;

    Console::setCursorPosition(x + 2, menuY);
    Console::print("2. My Portfolio");
    menuY += 1;

    Console::setCursorPosition(x + 2, menuY);
    Console::print("3. News");
    menuY += 1;

    Console::setCursorPosition(x + 2, menuY);
    Console::print("4. Financial Instruments");
    menuY += 1;

    Console::setCursorPosition(x + 2, menuY);
    Console::print("5. Save Game");
    menuY += 1;

    Console::setCursorPosition(x + 2, menuY);
    Console::print("9. Advance to Next Day");
    menuY += 1;

    Console::setCursorPosition(x + 2, menuY);
    Console::print("0. Exit Game");
    menuY += 2;

    Console::setCursorPosition(x + 2, menuY - 1);
    Console::print("Choose action: ");
}

bool MainScreen::handleInput(int key) {
    switch (key) {
        case '1':
            openMarketScreen();
            return true;

        case '2':
            openPortfolioScreen();
            return true;

        case '3':
            openNewsScreen();
            return true;

        case '4':
            openFinancialScreen();
            return true;

        case '5':
            saveGame();
            return true;

        case '9':
            advanceDay();
            return true;

        case '0':
        case 27:
            close();
            return false;

        default:
            return Screen::handleInput(key);
    }
}

void MainScreen::openMarketScreen() const {
    auto marketScreen = std::make_shared<MarketScreen>();
    marketScreen->setMarket(market);
    marketScreen->setPlayer(player);
    marketScreen->setNewsService(newsService);
    Console::clear();
    marketScreen->setPosition(x, y);

    marketScreen->initialize();
    marketScreen->run();
    Console::clear();
    draw();
}

void MainScreen::openPortfolioScreen() const {
    auto portfolioScreen = std::make_shared<PortfolioScreen>();
    portfolioScreen->setMarket(market);
    portfolioScreen->setPlayer(player);
    Console::clear();
    portfolioScreen->setPosition(x, y);

    portfolioScreen->initialize();
    portfolioScreen->run();
    Console::clear();
    draw();
}

void MainScreen::openNewsScreen() const {
    auto newsScreen = std::make_shared<NewsScreen>();
    newsScreen->setMarket(market);
    newsScreen->setPlayer(player);
    newsScreen->setNewsService(newsService);
    Console::clear();
    newsScreen->setPosition(x, y);

    newsScreen->initialize();
    newsScreen->run();
    Console::clear();
    draw();
}

void MainScreen::openFinancialScreen() const {
    auto financialScreen = std::make_shared<FinancialScreen>();
    financialScreen->setMarket(market);
    financialScreen->setPlayer(player);
    Console::clear();
    financialScreen->setPosition(x, y);

    financialScreen->initialize();
    financialScreen->run();
    Console::clear();
    draw();
}

void MainScreen::saveGame() const {
    if (!game) {
        Console::setCursorPosition(x + 2, y + 30);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("Error: Game not initialized!");
        Console::sleep(1500);
        return;
    }

    auto playerPtr = player.lock();
    if (!playerPtr) {
        Console::setCursorPosition(x + 2, y + 30);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("Error: Player not available!");
        Console::sleep(1500);
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");

    std::string saveName = playerPtr->getName() + "_Day" +
                          std::to_string(playerPtr->getCurrentDay()) + "_" +
                          ss.str();

    if (game->saveGame(saveName)) {
        Console::setCursorPosition(x + 2, y + 30);
        Console::setColor(TextColor::Green, bodyBg);
        Console::print("Game saved successfully!");
    } else {
        Console::setCursorPosition(x + 2, y + 30);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("Error saving game: " + game->getLastError());
    }

    Console::sleep(1500);
    Console::clear();
}

void MainScreen::advanceDay() {
    if (!game) {
        Console::setCursorPosition(x + 2, y + 30);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("Error: Game not initialized!");
        Console::sleep(1500);
        return;
    }

    if (game->getStatus() != GameStatus::Running) {
        if (!game->start()) {
            Console::setCursorPosition(x + 2, y + 30);
            Console::setColor(TextColor::Red, bodyBg);
            Console::print("Error starting game: " + game->getLastError());
            Console::sleep(1500);
            return;
        }
    }

    if (game->simulateDay()) {
        Console::setCursorPosition(x + 2, y + 30);
        Console::setColor(TextColor::Green, bodyBg);
        Console::print("Advanced to next day!");

        update();
    } else {
        Console::setCursorPosition(x + 2, y + 30);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("Error advancing day: " + game->getLastError());
    }

    Console::sleep(1500);
}

}