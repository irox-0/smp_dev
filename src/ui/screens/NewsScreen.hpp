#pragma once

#include "../Screen.hpp"
#include "../../models/News.hpp"
#include "../../services/NewsService.hpp"
#include "../../ui/widgets/Table.hpp"
#include <vector>
#include <memory>

namespace StockMarketSimulator {

enum class NewsFilter {
    All,
    Global,
    Sector,
    Corporate
};

class NewsScreen : public Screen {
private:
    std::weak_ptr<NewsService> newsService;

    NewsFilter currentFilter;
    int currentPage;
    int newsPerPage;
    std::vector<News> displayedNews;

    void updateDisplayedNews();
    void displayNewsDetails(const News& news);
    void changeFilter();
    void previousPage();
    void nextPage();

    void drawFilterInfo() const;
    void drawNewsList() const;
    void drawNavigationOptions() const;

protected:
    virtual void drawContent() const override;

public:
    NewsScreen();

    void setNewsService(std::weak_ptr<NewsService> newsService);

    virtual void initialize() override;
    virtual void update() override;
    virtual bool handleInput(int key) override;

    NewsFilter getCurrentFilter() const;
    void setCurrentFilter(NewsFilter filter);

    int getCurrentPage() const;
    void setCurrentPage(int page);

    int getNewsPerPage() const;
    void setNewsPerPage(int count);

    int getTotalPages() const;
};

}