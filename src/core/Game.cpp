#include "Game.hpp"
#include <algorithm>
#include <stdexcept>

namespace StockMarketSimulator {

Game::Game()
    : status(GameStatus::NotStarted),
      gameSpeed(1),
      simulatedDays(0),
      startDate(Date(1, 3, 2023))
{
}

void Game::initialize(const std::string& playerName, double initialBalance) {
    try {
        FileIO::clearLog(); // Clear the log file at the start
        FileIO::appendToLog("Game initialization started");
        market = std::make_shared<Market>();
        market->addDefaultCompanies();

        newsService = std::make_shared<NewsService>(market);
        newsService->initialize();
        newsService->setCurrentDate(startDate);

        priceService = std::make_shared<PriceService>(market);
        priceService->initialize();

        // Initialize dividend schedules for all companies
        for (const auto& company : market->getCompanies()) {
            company->initializeDividendSchedule(startDate);
        }

        for (int i = 0; i < 5; i++) {
            auto news = newsService->generateDailyNews(2);
            newsService->applyNewsEffects(news);
            market->simulateDay();
        }

        player = std::make_shared<Player>(playerName, initialBalance);
        player->setMarket(market);
        player->setCurrentDate(market->getCurrentDate());

        saveService = std::make_shared<SaveService>(market, player, newsService, priceService);
        saveService->initialize();

        status = GameStatus::NotStarted;
        simulatedDays = 0;
        lastError = "";
    } catch (const std::exception& e) {
        lastError = "Initialization error: " + std::string(e.what());
        status = GameStatus::Ended;
    }
}
bool Game::start() {
    if (status == GameStatus::NotStarted || status == GameStatus::Paused) {
        status = GameStatus::Running;
        return true;
    }
    
    lastError = "Cannot start game: invalid state";
    return false;
}

void Game::pause() {
    if (status == GameStatus::Running) {
        status = GameStatus::Paused;
    }
}

void Game::resume() {
    if (status == GameStatus::Paused) {
        status = GameStatus::Running;
    }
}

void Game::end() {
    status = GameStatus::Ended;
}

bool Game::simulateDay() {
    if (status != GameStatus::Running) {
        lastError = "Cannot simulate day: game is not running";
        return false;
    }

    try {
        auto dailyNews = newsService->generateDailyNews();

        if (priceService) {
            priceService->updatePrices();
        }

        market->simulateDay();

        newsService->applyNewsEffects(dailyNews);

        // Process dividends and notify player
        auto dividendPayments = market->processCompanyDividends();

        std::stringstream logMsg;
        logMsg << "Day " << simulatedDays << " - Processing "
               << dividendPayments.size() << " dividend payments";
        FileIO::appendToLog(logMsg.str());
        for (const auto& [company, amount] : dividendPayments) {
            player->receiveDividends(company, amount);
        }

        player->updateDailyState();
        player->closeDay();

        if (saveService) {
            saveService->checkAndCreateAutosave();
        }

        simulatedDays++;
        return true;
    } catch (const std::exception& e) {
        lastError = "Error during day simulation: " + std::string(e.what());
        return false;
    }
}
bool Game::simulateDays(int days) {
    if (days <= 0) {
        return true;
    }
    
    for (int i = 0; i < days; i++) {
        if (!simulateDay()) {
            return false;
        }
    }
    
    return true;
}

std::shared_ptr<Market> Game::getMarket() const {
    return market;
}

std::shared_ptr<Player> Game::getPlayer() const {
    return player;
}

std::shared_ptr<NewsService> Game::getNewsService() const {
    return newsService;
}

std::shared_ptr<PriceService> Game::getPriceService() const {
    return priceService;
}

std::shared_ptr<SaveService> Game::getSaveService() const {
    return saveService;
}

GameStatus Game::getStatus() const {
    return status;
}

int Game::getGameSpeed() const {
    return gameSpeed;
}

void Game::setGameSpeed(int speed) {
    if (speed > 0) {
        gameSpeed = speed;
    }
}

int Game::getSimulatedDays() const {
    return simulatedDays;
}

Date Game::getStartDate() const {
    return startDate;
}

bool Game::saveGame(const std::string& displayName) {
    if (!saveService) {
        lastError = "Save service not initialized";
        return false;
    }
    
    return saveService->saveGame(displayName);
}

bool Game::loadGame(const std::string& filename) {
    if (!saveService) {
        lastError = "Save service not initialized";
        return false;
    }
    
    bool result = saveService->loadGame(filename);
    if (result) {
        Date currentDate = player->getCurrentDate();
        if (newsService) {
            newsService->setCurrentDate(currentDate);
        }
    }
    
    return result;
}

std::string Game::getLastError() const {
    return lastError;
}

bool Game::buyStock(const std::string& ticker, int quantity, bool useMargin) {
    if (!player || !market) {
        lastError = "Player or market not initialized";
        return false;
    }
    
    auto company = market->getCompanyByTicker(ticker);
    if (!company) {
        lastError = "Company not found: " + ticker;
        return false;
    }
    
    return player->buyStock(company, quantity, useMargin);
}

bool Game::sellStock(const std::string& ticker, int quantity) {
    if (!player || !market) {
        lastError = "Player or market not initialized";
        return false;
    }
    
    auto company = market->getCompanyByTicker(ticker);
    if (!company) {
        lastError = "Company not found: " + ticker;
        return false;
    }
    
    return player->sellStock(company, quantity);
}

bool Game::takeLoan(double amount, double interestRate, int durationDays, const std::string& description) {
    if (!player) {
        lastError = "Player not initialized";
        return false;
    }
    
    return player->takeLoan(amount, interestRate, durationDays, description);
}

bool Game::repayLoan(size_t loanIndex, double amount) {
    if (!player) {
        lastError = "Player not initialized";
        return false;
    }
    
    return player->repayLoan(loanIndex, amount);
}

}