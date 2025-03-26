#pragma once

#include "../Screen.hpp"
#include "../../models/Company.hpp"
#include "../../ui/widgets/Table.hpp"
#include "../../core/Market.hpp"
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
    };

}