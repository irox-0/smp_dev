#pragma once

#include "../Screen.hpp"
#include "../../models/Company.hpp"
#include "../../ui/widgets/Table.hpp"
#include "CompanyScreen.hpp"
#include "../../services/NewsService.hpp"
#include <vector>
#include <memory>

namespace StockMarketSimulator {

    enum class MarketSortCriteria {
        Name,
        Price,
        PriceChange,
        PriceChangePercent,
        Sector
    };

    class MarketScreen : public Screen {
    private:
        std::vector<std::shared_ptr<Company>> displayedCompanies;
        Table companiesTable;
        MarketSortCriteria sortCriteria;
        bool sortAscending;
        std::weak_ptr<NewsService> newsService;

        void updateDisplayedCompanies();
        void updateTableData();
        void sortCompanies();
        void viewCompanyDetails(int index);
        void drawSortInfo() const;
        void drawNavigationOptions() const;

    protected:
        virtual void drawContent() const override;

    public:
        MarketScreen();

        virtual void initialize() override;
        virtual void update() override;
        virtual bool handleInput(int key) override;

        MarketSortCriteria getSortCriteria() const;
        void setSortCriteria(MarketSortCriteria criteria);

        bool isSortAscending() const;
        void setSortAscending(bool ascending);

        void changeSortCriteria();
        void toggleSortDirection();

        void setNewsService(std::weak_ptr<NewsService> newsService);
        std::weak_ptr<NewsService> getNewsService() const;

        const std::vector<std::shared_ptr<Company>>& getCompanies() const;
    };

}