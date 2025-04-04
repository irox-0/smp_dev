#pragma once

#include "../Screen.hpp"
#include "../../models/Company.hpp"
#include "../../core/Game.hpp"
#include "../../services/NewsService.hpp"
#include "MarketScreen.hpp"
#include "PortfolioScreen.hpp"
#include "NewsScreen.hpp"
#include "FinancialScreen.hpp"
#include <memory>
#include <vector>

namespace StockMarketSimulator {

    class MainScreen : public Screen {
    private:
        std::shared_ptr<Game> game;
        std::weak_ptr<NewsService> newsService;
        std::vector<std::shared_ptr<Company>> topStocks;
        std::vector<News> latestNews;

        void drawDateAndMarket() const;
        void drawPlayerInfo() const;
        void drawTopStocks() const;
        void drawLatestNews() const;
        void drawNavigationMenu() const;

        void updateTopStocks();
        void updateLatestNews();
        void checkGameOverConditions();
        void gameOver(const std::string& message);

        void openMarketScreen() const;
        void openPortfolioScreen() const;
        void openNewsScreen() const;
        void openFinancialScreen() const;
        void saveGame() const;
        void advanceDay();

    protected:
        virtual void drawContent() const override;

    public:
        MainScreen();

        void setGame(std::shared_ptr<Game> game);
        std::shared_ptr<Game> getGame() const;

        void setNewsService(std::weak_ptr<NewsService> newsService);
        std::weak_ptr<NewsService> getNewsService() const;

        virtual void initialize() override;
        virtual void update() override;
        virtual bool handleInput(int key) override;
    };

}