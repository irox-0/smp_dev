#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <nlohmann/json.hpp>
#include "../models/News.hpp"
#include "../models/Company.hpp"
#include "../core/Market.hpp"
#include "../utils/Random.hpp"
#include "../utils/FileIO.hpp"
#include "../utils/Date.hpp"

namespace StockMarketSimulator {

class NewsService {
private:
    std::weak_ptr<Market> market;
    std::vector<News> newsHistory;
    std::vector<NewsTemplate> newsTemplates;
    std::map<NewsType, std::vector<NewsTemplate>> categoryTemplates;

    Date currentDate;
    int newsPerDay;

    void loadNewsTemplates(const std::string& filePath);

    void createDefaultTemplates();

    News generateRandomNews(NewsType type);

    std::string processTemplate(const std::string& templ,
                                const std::shared_ptr<Company>& company = nullptr);

    NewsTemplate selectNewsTemplate(NewsType type, MarketTrend trend);

public:
    NewsService();
    NewsService(std::weak_ptr<Market> market);

    void initialize(const std::string& templatesPath = "data/news_templates.json");

    void setMarket(std::weak_ptr<Market> market);

    const std::vector<News>& getNewsHistory() const;

    std::vector<News> getNewsByDay(int day) const;

    std::vector<News> getLatestNews(int count = 5) const;

    std::vector<News> generateDailyNews(int newsCount = 0);

    void applyNewsEffects(const std::vector<News>& news);

    void addCustomNews(const News& news);

    Date getCurrentDate() const;
    void setCurrentDate(const Date& date);
    
    int getNewsPerDay() const;
    void setNewsPerDay(int count);
    
    nlohmann::json toJson() const;
    static NewsService fromJson(const nlohmann::json& json, std::weak_ptr<Market> market);
};

}