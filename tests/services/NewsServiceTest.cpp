#include <gtest/gtest.h>
#include "../../src/services/NewsService.hpp"
#include "../../src/core/Market.hpp"
#include "../../src/utils/Date.hpp"
#include "../../src/utils/Random.hpp"
#include <memory>
#include <algorithm>

namespace StockMarketSimulator {

class NewsServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize with a fixed seed for deterministic tests
        Random::initialize(42);

        market = std::make_shared<Market>();
        market->addDefaultCompanies();

        newsService = std::make_shared<NewsService>(market);
        newsService->initialize();
    }

    std::shared_ptr<Market> market;
    std::shared_ptr<NewsService> newsService;
};

// Test Class Construction
TEST_F(NewsServiceTest, DefaultConstructor) {
    NewsService defaultService;
    EXPECT_EQ(defaultService.getCurrentDate(), Date(1, 3, 2023));
    EXPECT_EQ(defaultService.getNewsPerDay(), 2);
    EXPECT_TRUE(defaultService.getNewsHistory().empty());
}

TEST_F(NewsServiceTest, ConstructorWithMarket) {
    EXPECT_EQ(newsService->getCurrentDate(), Date(1, 3, 2023));
    EXPECT_EQ(newsService->getNewsPerDay(), 2);
    EXPECT_TRUE(newsService->getNewsHistory().empty());
}

// Test Initialization, Loading Templates, and Default Templates
TEST_F(NewsServiceTest, InitializeLoadsTemplates) {
    // We can't directly check if templates are loaded since they're private
    // But we can check if generateDailyNews produces a variety of news types
    auto news = newsService->generateDailyNews(10);
    ASSERT_GE(news.size(), 5);

    // Check we get different news titles
    std::set<std::string> titles;
    for (const auto& item : news) {
        titles.insert(item.getTitle());
    }
    EXPECT_GE(titles.size(), 3); // Should have at least 3 different titles
}

// Test Setting and Getting Basic Properties
TEST_F(NewsServiceTest, SetGetNewsPerDay) {
    newsService->setNewsPerDay(5);
    EXPECT_EQ(newsService->getNewsPerDay(), 5);

    // Test with invalid value
    newsService->setNewsPerDay(0);
    EXPECT_EQ(newsService->getNewsPerDay(), 5); // Should remain unchanged

    newsService->setNewsPerDay(-1);
    EXPECT_EQ(newsService->getNewsPerDay(), 5); // Should remain unchanged
}

TEST_F(NewsServiceTest, SetGetCurrentDate) {
    Date testDate(15, 4, 2023);
    newsService->setCurrentDate(testDate);
    EXPECT_EQ(newsService->getCurrentDate(), testDate);
}

// Test News Generation
TEST_F(NewsServiceTest, GenerateDailyNews) {
    // Test with default count
    auto news1 = newsService->generateDailyNews();
    EXPECT_EQ(news1.size(), 2); // Default is 2 news per day

    // Test with explicit count
    auto news2 = newsService->generateDailyNews(4);
    EXPECT_EQ(news2.size(), 4);

    // Test news are added to history
    EXPECT_EQ(newsService->getNewsHistory().size(), news1.size() + news2.size());
}

TEST_F(NewsServiceTest, GeneratedNewsTypes) {
    // Generate many news to ensure we get all types
    auto news = newsService->generateDailyNews(20);

    bool hasGlobal = false;
    bool hasSector = false;
    bool hasCorporate = false;

    for (const auto& item : news) {
        if (item.getType() == NewsType::Global) hasGlobal = true;
        if (item.getType() == NewsType::Sector) hasSector = true;
        if (item.getType() == NewsType::Corporate) hasCorporate = true;
    }

    EXPECT_TRUE(hasGlobal) << "No global news was generated";
    EXPECT_TRUE(hasSector) << "No sector news was generated";
    EXPECT_TRUE(hasCorporate) << "No corporate news was generated";
}

TEST_F(NewsServiceTest, GeneratedNewsHasCorrectDate) {
    Date testDate(10, 5, 2023);
    newsService->setCurrentDate(testDate);

    auto news = newsService->generateDailyNews(5);

    for (const auto& item : news) {
        EXPECT_EQ(item.getPublishDate(), testDate);
    }
}

TEST_F(NewsServiceTest, NewsHistoryMaxSize) {
    // Generate more than the max history size to test trimming
    for (int i = 0; i < 1200; i++) {
        newsService->generateDailyNews(1);
    }

    // Max size is 1000 as defined in NewsService.cpp
    EXPECT_LE(newsService->getNewsHistory().size(), 1000);
}

// Test News Retrieval Methods
TEST_F(NewsServiceTest, GetNewsByDay) {
    // Add news with different dates
    Date day1(1, 6, 2023);
    Date day2(2, 6, 2023);

    News news1(NewsType::Global, "Day 1 News", "Content 1", 0.01, day1);
    News news2(NewsType::Global, "Day 2 News A", "Content 2A", 0.02, day2);
    News news3(NewsType::Global, "Day 2 News B", "Content 2B", 0.02, day2);

    newsService->addCustomNews(news1);
    newsService->addCustomNews(news2);
    newsService->addCustomNews(news3);

    // Get news by day
    int day1Number = day1.toDayNumber();
    int day2Number = day2.toDayNumber();

    auto day1News = newsService->getNewsByDay(day1Number);
    auto day2News = newsService->getNewsByDay(day2Number);

    EXPECT_EQ(day1News.size(), 1);
    EXPECT_EQ(day1News[0].getTitle(), "Day 1 News");

    EXPECT_EQ(day2News.size(), 2);
    // The titles might be in any order, so we'll check both are present
    std::vector<std::string> day2Titles;
    for (const auto& news : day2News) {
        day2Titles.push_back(news.getTitle());
    }
    EXPECT_TRUE(std::find(day2Titles.begin(), day2Titles.end(), "Day 2 News A") != day2Titles.end());
    EXPECT_TRUE(std::find(day2Titles.begin(), day2Titles.end(), "Day 2 News B") != day2Titles.end());
}

TEST_F(NewsServiceTest, GetLatestNews) {
    // Add a series of news
    for (int i = 0; i < 10; i++) {
        News news(NewsType::Global, "News " + std::to_string(i), "Content", 0.01, Date(i + 1, 6, 2023));
        newsService->addCustomNews(news);
    }

    // Get latest 5 news
    auto latestNews = newsService->getLatestNews(5);
    EXPECT_EQ(latestNews.size(), 5);

    // News are returned in the order they are in the history (most recent additions last)
    // Last 5 news added were News 5 to News 9
    std::vector<std::string> expectedTitles;
    for (int i = 5; i < 10; i++) {
        expectedTitles.push_back("News " + std::to_string(i));
    }

    for (size_t i = 0; i < latestNews.size(); i++) {
        EXPECT_EQ(latestNews[i].getTitle(), expectedTitles[i]);
    }

    // Test with count larger than history
    auto allNews = newsService->getLatestNews(20);
    EXPECT_EQ(allNews.size(), 10);
}

// Test Adding Custom News
TEST_F(NewsServiceTest, AddCustomNews) {
    News customNews(NewsType::Corporate, "Custom News", "This is custom news", 0.03, Date(15, 6, 2023));
    newsService->addCustomNews(customNews);

    const auto& history = newsService->getNewsHistory();
    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].getTitle(), "Custom News");
    EXPECT_EQ(history[0].getContent(), "This is custom news");
    EXPECT_EQ(history[0].getImpact(), 0.03);
    EXPECT_EQ(history[0].getPublishDate(), Date(15, 6, 2023));
}

// Test Applying News Effects
TEST_F(NewsServiceTest, ApplyNewsEffectsToMarket) {
    // Create global news that affects the market
    News marketNews(NewsType::Global, "Economic News", "Major economic event", 0.05, Date(1, 7, 2023));

    // Make a copy for applying effects since the original won't be modified
    std::vector<News> newsToApply = {marketNews};

    double initialIndex = market->getMarketIndex();

    newsService->applyNewsEffects(newsToApply);

    // Market index should have changed
    EXPECT_NE(market->getMarketIndex(), initialIndex);

    // The processed flag is set on the copy, not the original
    EXPECT_TRUE(newsToApply[0].isProcessed());
}

TEST_F(NewsServiceTest, ApplyNewsEffectsToSector) {
    // Get companies in technology sector
    auto techCompanies = market->getCompaniesBySector(Sector::Technology);
    ASSERT_FALSE(techCompanies.empty());

    // Store initial prices
    std::vector<double> initialPrices;
    for (const auto& company : techCompanies) {
        initialPrices.push_back(company->getStock()->getCurrentPrice());
    }

    // Create sector news
    News sectorNews(NewsType::Sector, "Technology News", "Important tech development", 0.05, Date(1, 7, 2023));
    sectorNews.setTargetSector(Sector::Technology);

    // Make a copy for applying effects
    std::vector<News> newsToApply = {sectorNews};

    newsService->applyNewsEffects(newsToApply);

    // Check prices changed for tech companies
    bool anyPriceChanged = false;
    for (size_t i = 0; i < techCompanies.size(); i++) {
        double newPrice = techCompanies[i]->getStock()->getCurrentPrice();
        if (std::abs(newPrice - initialPrices[i]) > 0.0001) { // Using epsilon for float comparison
            anyPriceChanged = true;
            break;
        }
    }

    EXPECT_TRUE(anyPriceChanged);
    EXPECT_TRUE(newsToApply[0].isProcessed());
}

TEST_F(NewsServiceTest, ApplyNewsEffectsToCompany) {
    // Get a specific company
    auto companies = market->getCompanies();
    ASSERT_FALSE(companies.empty());
    auto targetCompany = companies[0];

    // Save initial price as string for better debugging
    double initialPrice = targetCompany->getStock()->getCurrentPrice();
    std::string initialPriceStr = std::to_string(initialPrice);

    // Create company-specific news with significant impact
    News companyNews(NewsType::Corporate, "Company News", "Company specific event", 0.1, Date(1, 7, 2023));
    companyNews.setTargetCompany(targetCompany);

    // Make a copy for applying effects
    std::vector<News> newsToApply = {companyNews};

    newsService->applyNewsEffects(newsToApply);

    // Check price changed for the target company
    double newPrice = targetCompany->getStock()->getCurrentPrice();
    std::string newPriceStr = std::to_string(newPrice);

    EXPECT_TRUE(std::abs(newPrice - initialPrice) > 0.0001)
        << "Price didn't change. Initial: " << initialPriceStr
        << ", New: " << newPriceStr;

    EXPECT_TRUE(newsToApply[0].isProcessed());
}

TEST_F(NewsServiceTest, ApplyNewsEffectsSkipProcessed) {
    // Create pre-processed news
    News processedNews(NewsType::Global, "Old News", "Already processed", 0.05, Date(1, 7, 2023));
    processedNews.setProcessed(true);
    std::vector<News> news = {processedNews};

    double initialIndex = market->getMarketIndex();

    newsService->applyNewsEffects(news);

    // Market index should not have changed
    EXPECT_EQ(market->getMarketIndex(), initialIndex);
}

// Test Serialization
TEST_F(NewsServiceTest, SerializationAndDeserialization) {
    // Set up a service with some news
    Date testDate(20, 7, 2023);
    newsService->setCurrentDate(testDate);
    newsService->setNewsPerDay(3);

    // Add custom news
    News customNews(NewsType::Global, "Serialization Test", "Test content", 0.02, testDate);
    newsService->addCustomNews(customNews);

    // Generate some news
    auto generatedNews = newsService->generateDailyNews(2);

    // Serialize
    nlohmann::json jsonData = newsService->toJson();

    // Verify JSON structure
    EXPECT_TRUE(jsonData.contains("current_date"));
    EXPECT_TRUE(jsonData.contains("news_per_day"));
    EXPECT_TRUE(jsonData.contains("news_history"));
    EXPECT_EQ(jsonData["news_per_day"], 3);
    EXPECT_EQ(jsonData["news_history"].size(), 3); // 1 custom + 2 generated

    // Create a new service from the JSON
    auto newService = NewsService::fromJson(jsonData, market);

    // Check basic properties
    EXPECT_EQ(newService.getCurrentDate(), testDate);
    EXPECT_EQ(newService.getNewsPerDay(), 3);
    EXPECT_EQ(newService.getNewsHistory().size(), 3);

    // Find our custom news in the deserialized history
    bool foundCustomNews = false;
    for (const auto& news : newService.getNewsHistory()) {
        if (news.getTitle() == "Serialization Test") {
            EXPECT_EQ(news.getContent(), "Test content");
            EXPECT_EQ(news.getImpact(), 0.02);
            EXPECT_EQ(news.getPublishDate(), testDate);
            foundCustomNews = true;
            break;
        }
    }
    EXPECT_TRUE(foundCustomNews);
}

TEST_F(NewsServiceTest, BackwardCompatibilityFromJson) {
    // Create old format JSON
    nlohmann::json oldFormatJson;

    // Create a day number (e.g., 30 days from the reference date)
    int testDay = 30;
    Date expectedDate = Date::fromDayNumber(testDay);

    oldFormatJson["current_day"] = testDay;
    oldFormatJson["news_per_day"] = 2;
    oldFormatJson["news_history"] = nlohmann::json::array();

    // Add some news to the history
    nlohmann::json newsItem;
    newsItem["type"] = "Global";
    newsItem["title"] = "Old Format News";
    newsItem["content"] = "Old format content";
    newsItem["impact"] = 0.03;
    newsItem["publish_day"] = testDay;
    newsItem["processed"] = false;
    newsItem["target_sector"] = "Unknown";
    newsItem["target_company_ticker"] = "";

    oldFormatJson["news_history"].push_back(newsItem);

    // Deserialize from old format JSON
    auto newService = NewsService::fromJson(oldFormatJson, market);

    // Verify the day was converted to the correct date
    EXPECT_EQ(newService.getCurrentDate(), expectedDate);

    // Verify the news history was loaded
    const auto& history = newService.getNewsHistory();
    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].getTitle(), "Old Format News");
    EXPECT_EQ(history[0].getPublishDate(), expectedDate);
}

TEST_F(NewsServiceTest, OperationsWithEmptyMarket) {
    // Create a service with empty market - fix the syntax here
    std::weak_ptr<Market> emptyMarket; // Create empty weak_ptr this way
    NewsService emptyMarketService(emptyMarket);
    emptyMarketService.initialize();

    // Try generating news
    auto news = emptyMarketService.generateDailyNews();
    EXPECT_TRUE(news.empty());

    // Try applying effects (should not crash)
    News testNews(NewsType::Global, "Test", "Content", 0.01, Date(1, 1, 2023));
    std::vector<News> testNewsVector = {testNews};
    emptyMarketService.applyNewsEffects(testNewsVector);

    // Deserialize with empty market
    nlohmann::json jsonData = emptyMarketService.toJson();
    auto deserializedService = NewsService::fromJson(jsonData, emptyMarket);
    EXPECT_TRUE(deserializedService.getNewsHistory().empty());
}

} // namespace StockMarketSimulator