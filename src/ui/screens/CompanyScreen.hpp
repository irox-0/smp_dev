#pragma once

#include "../Screen.hpp"
#include "../../models/Company.hpp"
#include "../../ui/widgets/Chart.hpp"
#include "../../services/NewsService.hpp"
#include <memory>
#include <vector>


namespace StockMarketSimulator {

    class CompanyScreen : public Screen {
    private:
        std::shared_ptr<Company> company;
        Chart priceChart;
        std::weak_ptr<NewsService> newsService;

        void drawCompanyInfo() const;
        void drawPriceChart() const;
        void drawLatestNews() const;
        void drawActions() const;
        void drawDescription() const;

        void buyStocks();
        void sellStocks();


    protected:
        virtual void drawContent() const override;

    public:
        CompanyScreen();

        void setCompany(std::shared_ptr<Company> company);
        std::shared_ptr<Company> getCompany() const;

        void setNewsService(std::weak_ptr<NewsService> newsService);

        virtual void initialize() override;
        virtual void update() override;
        virtual bool handleInput(int key) override;

    };

}