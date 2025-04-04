#pragma once

#include <string>
#include <memory>
#include <vector>
#include "Market.hpp"
#include "Player.hpp"
#include "../services/NewsService.hpp"
#include "../services/PriceService.hpp"
#include "../services/SaveService.hpp"

namespace StockMarketSimulator {

enum class GameStatus {
    NotStarted,
    Running,
    Paused,
    Ended
};

class Game : public std::enable_shared_from_this<Game> {
private:
    std::shared_ptr<Market> market;
    std::shared_ptr<Player> player;
    std::shared_ptr<NewsService> newsService;
    std::shared_ptr<PriceService> priceService;
    std::shared_ptr<SaveService> saveService;

    GameStatus status;
    int gameSpeed;
    int simulatedDays;
    Date startDate;
    std::string lastError;

public:
    Game();

    void initialize(const std::string& playerName = "Trader", double initialBalance = 10000.0);

    bool start();
    void pause();
    void resume();
    void end();

    bool simulateDay();
    bool simulateDays(int days);

    std::shared_ptr<Market> getMarket() const;
    std::shared_ptr<Player> getPlayer() const;
    std::shared_ptr<NewsService> getNewsService() const;
    std::shared_ptr<PriceService> getPriceService() const;
    std::shared_ptr<SaveService> getSaveService() const;

    GameStatus getStatus() const;
    int getGameSpeed() const;
    void setGameSpeed(int speed);
    int getSimulatedDays() const;
    Date getStartDate() const;

    bool saveGame(const std::string& displayName);
    bool loadGame(const std::string& filename);

    std::string getLastError() const;

    bool buyStock(const std::string& ticker, int quantity, bool useMargin = false);
    bool sellStock(const std::string& ticker, int quantity);
    bool takeLoan(double amount, double interestRate, int durationDays, const std::string& description = "Standard Loan");
    bool repayLoan(size_t loanIndex, double amount);
};

}