#pragma once

#include "../Screen.hpp"
#include "../../models/Portfolio.hpp"
#include "../../ui/widgets/Table.hpp"
#include "../../ui/widgets/Chart.hpp"
#include "CompanyScreen.hpp"
#include <memory>
#include <vector>

namespace StockMarketSimulator {

class PortfolioScreen : public Screen {
private:
    Table portfolioTable;
    Chart valueChart;
    int currentPage;
    int itemsPerPage;
    int totalPages;

    void drawPortfolioInfo() const;
    void drawPortfolioTable() const;
    void drawSectorDistribution() const;
    void drawValueChart() const;
    void drawNavigationOptions() const;

    void nextPage();
    void previousPage();
    void updateTableData();

    void viewStockDetails();
    void sellStocks();

protected:
    virtual void drawContent() const override;

public:
    PortfolioScreen();

    virtual void initialize() override;
    virtual void update() override;
    virtual bool handleInput(int key) override;
};

}