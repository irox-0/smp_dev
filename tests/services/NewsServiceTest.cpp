#include <gtest/gtest.h>
#include "services/NewsService.hpp"
#include "core/Market.hpp"
#include "models/News.hpp"
#include "models/Company.hpp"
#include "utils/Random.hpp"

using namespace StockMarketSimulator;

class NewsServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        Random::initialize(42);
        
        market = std::make_shared<Market>();
        market->addDefaultCompanies();
        
        newsService = std::make_unique<NewsService>(market);
        newsService->initialize();
    }
    
    std::shared_ptr<Market> market;
    std::unique_ptr<NewsService> newsService;
};

TEST_F(NewsServiceTest, InitializationTest) {
    ASSERT_EQ(newsService->getNewsHistory().size(), 0);
    
    NewsService emptyService;
    ASSERT_NO_THROW(emptyService.initialize());
}

TEST_F(NewsServiceTest, GenerateDailyNewsTest) {
    int newsCount = 3;
    auto news = newsService->generateDailyNews(newsCount);
    
    ASSERT_EQ(news.size(), newsCount);
    ASSERT_EQ(newsService->getNewsHistory().size(), newsCount);
    
    for (const auto& newsItem : news) {
        ASSERT_FALSE(newsItem.getTitle().empty());
        ASSERT_FALSE(newsItem.getContent().empty());
        ASSERT_NE(newsItem.getImpact(), 0.0);
        ASSERT_EQ(newsItem.getPublishDay(), 0);
        ASSERT_FALSE(newsItem.isProcessed());
    }
}

TEST_F(NewsServiceTest, GetNewsByDayTest) {
    newsService->setCurrentDay(1);
    newsService->generateDailyNews(2);
    
    newsService->setCurrentDay(2);
    newsService->generateDailyNews(3);
    
    auto day1News = newsService->getNewsByDay(1);
    auto day2News = newsService->getNewsByDay(2);
    
    ASSERT_EQ(day1News.size(), 2);
    ASSERT_EQ(day2News.size(), 3);
    
    for (const auto& news : day1News) {
        ASSERT_EQ(news.getPublishDay(), 1);
    }
    
    for (const auto& news : day2News) {
        ASSERT_EQ(news.getPublishDay(), 2);
    }
}

TEST_F(NewsServiceTest, GetLatestNewsTest) {
    for (int day = 1; day <= 5; day++) {
        newsService->setCurrentDay(day);
        newsService->generateDailyNews(1);
    }
    
    auto latestNews = newsService->getLatestNews(3);
    
    ASSERT_EQ(latestNews.size(), 3);
    ASSERT_EQ(latestNews[0].getPublishDay(), 3);
    ASSERT_EQ(latestNews[1].getPublishDay(), 4);
    ASSERT_EQ(latestNews[2].getPublishDay(), 5);
}

TEST_F(NewsServiceTest, ApplyNewsEffectsTest) {
    auto techCompany = market->getCompanyByTicker("TCH");
    auto energyCompany = market->getCompanyByTicker("EPLC");
    
    ASSERT_NE(techCompany, nullptr);
    ASSERT_NE(energyCompany, nullptr);
    
    double initialMarketIndex = market->getMarketIndex();
    double initialTechPrice = techCompany->getStock()->getCurrentPrice();
    double initialEnergyPrice = energyCompany->getStock()->getCurrentPrice();
    
    std::vector<News> testNews;
    
    News globalNews(NewsType::Global, "Test Global News", "Test content", 0.02, 1);
    testNews.push_back(globalNews);
    
    News sectorNews(NewsType::Sector, "Test Sector News", "Test content", 0.03, 1, Sector::Technology);
    testNews.push_back(sectorNews);
    
    News corporateNews(NewsType::Corporate, "Test Company News", "Test content", 0.05, 1, techCompany);
    testNews.push_back(corporateNews);
    
    newsService->applyNewsEffects(testNews);
    
    double newMarketIndex = market->getMarketIndex();
    double newTechPrice = techCompany->getStock()->getCurrentPrice();
    double newEnergyPrice = energyCompany->getStock()->getCurrentPrice();
    
    ASSERT_GT(newMarketIndex, initialMarketIndex);
    
    ASSERT_GT(newTechPrice, initialTechPrice);
    
    ASSERT_NE(newEnergyPrice, initialEnergyPrice);
    ASSERT_LT(std::abs(newEnergyPrice - initialEnergyPrice), std::abs(newTechPrice - initialTechPrice));
    
    for (const auto& news : testNews) {
        ASSERT_TRUE(news.isProcessed());
    }
}

TEST_F(NewsServiceTest, AddCustomNewsTest) {
    News customNews(NewsType::Global, "Custom News", "Custom content", 0.01, 1);
    
    int initialCount = newsService->getNewsHistory().size();
    newsService->addCustomNews(customNews);
    int newCount = newsService->getNewsHistory().size();
    
    ASSERT_EQ(newCount, initialCount + 1);
    ASSERT_EQ(newsService->getNewsHistory().back().getTitle(), "Custom News");
}

TEST_F(NewsServiceTest, JsonSerializationTest) {
    newsService->setCurrentDay(1);
    newsService->generateDailyNews(2);
    
    newsService->setCurrentDay(2);
    newsService->generateDailyNews(3);
    
    nlohmann::json json = newsService->toJson();
    NewsService restoredService = NewsService::fromJson(json, market);
    
    ASSERT_EQ(restoredService.getCurrentDay(), 2);
    ASSERT_EQ(restoredService.getNewsHistory().size(), 5);
    
    auto latestNews = restoredService.getLatestNews(3);
    ASSERT_EQ(latestNews.size(), 3);
}

TEST_F(NewsServiceTest, MultipleDaysSimulationTest) {
    for (int day = 1; day <= 10; day++) {
        market->simulateDay();
        
        newsService->setCurrentDay(day);
        auto news = newsService->generateDailyNews(2);
        newsService->applyNewsEffects(news);
    }
    
    ASSERT_EQ(newsService->getNewsHistory().size(), 20);
    
    auto latestNews = newsService->getNewsByDay(10);
    ASSERT_EQ(latestNews.size(), 2);
    
    for (const auto& news : latestNews) {
        ASSERT_EQ(news.getPublishDay(), 10);
        ASSERT_TRUE(news.isProcessed());
    }
}

TEST_F(NewsServiceTest, GettersSettersTest) {
    newsService->setCurrentDay(5);
    ASSERT_EQ(newsService->getCurrentDay(), 5);
    
    newsService->setNewsPerDay(4);
    ASSERT_EQ(newsService->getNewsPerDay(), 4);
    
    auto news = newsService->generateDailyNews();
    ASSERT_EQ(news.size(), 4);
}

TEST_F(NewsServiceTest, EdgeCasesTest) {
    NewsService noMarketService;
    auto news = noMarketService.generateDailyNews();
    ASSERT_EQ(news.size(), 0);
    
    newsService->setCurrentDay(-5);
    ASSERT_NE(newsService->getCurrentDay(), -5);
    
    newsService->setNewsPerDay(-2);
    ASSERT_NE(newsService->getNewsPerDay(), -2);
    
    auto nonExistentNews = newsService->getNewsByDay(100);
    ASSERT_EQ(nonExistentNews.size(), 0);
}

TEST_F(NewsServiceTest, NewsCategoryEffectsTest) {
    auto techCompany = market->getCompanyByTicker("TCH");
    auto energyCompany = market->getCompanyByTicker("EPLC");
    
    News positiveGlobalNews(NewsType::Global, "Positive Global", "Content", 0.03, 1);
    News negativeGlobalNews(NewsType::Global, "Negative Global", "Content", -0.03, 1);
    
    News positiveTechNews(NewsType::Sector, "Positive Tech", "Content", 0.03, 1, Sector::Technology);
    News negativeEnergyNews(NewsType::Sector, "Negative Energy", "Content", -0.03, 1, Sector::Energy);
    
    News positiveTechCompanyNews(NewsType::Corporate, "Positive Tech Company", "Content", 0.03, 1, techCompany);
    News negativeEnergyCompanyNews(NewsType::Corporate, "Negative Energy Company", "Content", -0.03, 1, energyCompany);
    
    double initialMarketIndex = market->getMarketIndex();
    double initialTechPrice = techCompany->getStock()->getCurrentPrice();
    double initialEnergyPrice = energyCompany->getStock()->getCurrentPrice();
    
    std::vector<News> positiveNews = {positiveGlobalNews, positiveTechNews, positiveTechCompanyNews};
    newsService->applyNewsEffects(positiveNews);
    
    ASSERT_GT(market->getMarketIndex(), initialMarketIndex);
    ASSERT_GT(techCompany->getStock()->getCurrentPrice(), initialTechPrice);
    ASSERT_GT(energyCompany->getStock()->getCurrentPrice(), initialEnergyPrice);
    
    double midMarketIndex = market->getMarketIndex();
    double midTechPrice = techCompany->getStock()->getCurrentPrice();
    double midEnergyPrice = energyCompany->getStock()->getCurrentPrice();
    
    std::vector<News> negativeNews = {negativeGlobalNews, negativeEnergyNews, negativeEnergyCompanyNews};
    newsService->applyNewsEffects(negativeNews);
    
    ASSERT_LT(market->getMarketIndex(), midMarketIndex);
    ASSERT_LT(energyCompany->getStock()->getCurrentPrice(), midEnergyPrice);
}

TEST_F(NewsServiceTest, MarketTrendInfluenceTest) {

    market->setMarketTrend(MarketTrend::Bullish);
    newsService->setCurrentDay(1);

    auto bullishNews = newsService->generateDailyNews(5);

    int bullishPositiveCount = 0;
    int bullishNegativeCount = 0;

    for (const auto& news : bullishNews) {
        if (news.getImpact() > 0) bullishPositiveCount++;
        if (news.getImpact() < 0) bullishNegativeCount++;
    }

    market->setMarketTrend(MarketTrend::Bearish);
    newsService->setCurrentDay(2);

    auto bearishNews = newsService->generateDailyNews(5);

    int bearishPositiveCount = 0;
    int bearishNegativeCount = 0;

    for (const auto& news : bearishNews) {
        if (news.getImpact() > 0) bearishPositiveCount++;
        if (news.getImpact() < 0) bearishNegativeCount++;
    }


    std::cout << "Бычий рынок: " << bullishPositiveCount << " положительных, "
              << bullishNegativeCount << " отрицательных новостей." << std::endl;
    std::cout << "Медвежий рынок: " << bearishPositiveCount << " положительных, "
              << bearishNegativeCount << " отрицательных новостей." << std::endl;

    ASSERT_LE(bullishNegativeCount, bearishNegativeCount);
}

TEST_F(NewsServiceTest, LongTermSimulationTest) {
    const int SIMULATION_DAYS = 100;

    for (int day = 1; day <= SIMULATION_DAYS; day++) {
        newsService->setCurrentDay(day);
        auto news = newsService->generateDailyNews(3);
        newsService->applyNewsEffects(news);

        if (day % 30 == 0) {
            ASSERT_LE(newsService->getNewsHistory().size(), 1000);
        }
    }

    ASSERT_EQ(newsService->getNewsHistory().size(), 300);

    bool hasGlobalNews = false;
    bool hasSectorNews = false;
    bool hasCorporateNews = false;

    for (const auto& news : newsService->getNewsHistory()) {
        if (news.getType() == NewsType::Global) hasGlobalNews = true;
        if (news.getType() == NewsType::Sector) hasSectorNews = true;
        if (news.getType() == NewsType::Corporate) hasCorporateNews = true;

        if (hasGlobalNews && hasSectorNews && hasCorporateNews) break;
    }

    ASSERT_TRUE(hasGlobalNews);
    ASSERT_TRUE(hasSectorNews);
    ASSERT_TRUE(hasCorporateNews);
}

TEST_F(NewsServiceTest, SectorCoverageTest) {
    const int DAYS = 30;
    std::map<Sector, bool> sectorCoverage;

    for (int day = 1; day <= DAYS; day++) {
        newsService->setCurrentDay(day);
        auto news = newsService->generateDailyNews(3);

        for (const auto& newsItem : news) {
            if (newsItem.getType() == NewsType::Sector) {
                sectorCoverage[newsItem.getTargetSector()] = true;
            }
        }
    }

    ASSERT_TRUE(sectorCoverage[Sector::Technology]);
    ASSERT_TRUE(sectorCoverage[Sector::Energy]);
    ASSERT_TRUE(sectorCoverage[Sector::Finance]);
    ASSERT_TRUE(sectorCoverage[Sector::Consumer]);
    ASSERT_TRUE(sectorCoverage[Sector::Manufacturing]);
}

TEST_F(NewsServiceTest, WeakPtrTest) {
    auto weakMarket = std::weak_ptr<Market>();

    {
        auto tempMarket = std::make_shared<Market>();
        tempMarket->addDefaultCompanies();

        NewsService tempService(tempMarket);
        tempService.initialize();
        auto news = tempService.generateDailyNews(2);
        ASSERT_EQ(news.size(), 2);

        weakMarket = tempMarket;
    }

    NewsService invalidService(weakMarket);
    ASSERT_NO_THROW(invalidService.initialize());

    auto news = invalidService.generateDailyNews(2);
    ASSERT_EQ(news.size(), 0);

    ASSERT_NO_THROW(invalidService.applyNewsEffects({}));
}

TEST_F(NewsServiceTest, TemplateProcessingTest) {

    newsService->setCurrentDay(1);

    Random::initialize(42);

    auto generatedNews = newsService->generateDailyNews(15);

    News* globalNews = nullptr;
    for (auto& news : generatedNews) {
        if (news.getType() == NewsType::Global) {
            globalNews = &news;
            break;
        }
    }

    News* corporateNews = nullptr;
    for (auto& news : generatedNews) {
        if (news.getType() == NewsType::Corporate) {
            auto company = news.getTargetCompany().lock();
            if (company) {
                corporateNews = &news;
                break;
            }
        }
    }

    if (globalNews) {
        std::string title = globalNews->getTitle();
        std::string content = globalNews->getContent();

        ASSERT_EQ(title.find("%s"), std::string::npos);
        ASSERT_EQ(title.find("%d"), std::string::npos);
        ASSERT_EQ(content.find("%s"), std::string::npos);
        ASSERT_EQ(content.find("%d"), std::string::npos);
    }

    if (corporateNews) {
        std::string title = corporateNews->getTitle();
        std::string content = corporateNews->getContent();

        auto company = corporateNews->getTargetCompany().lock();
        if (company) {
            std::string companyName = company->getName();

            bool companyNameInTitle = title.find(companyName) != std::string::npos;
            bool companyNameInContent = content.find(companyName) != std::string::npos;

            ASSERT_TRUE(companyNameInTitle || companyNameInContent);
        }
    }

    ASSERT_TRUE(globalNews != nullptr || corporateNews != nullptr);
}

TEST_F(NewsServiceTest, NewsPerDaySetting) {

    ASSERT_EQ(newsService->getNewsPerDay(), 2);

    newsService->setNewsPerDay(5);
    ASSERT_EQ(newsService->getNewsPerDay(), 5);

    auto news = newsService->generateDailyNews();
    ASSERT_EQ(news.size(), 5);

    news = newsService->generateDailyNews(3);
    ASSERT_EQ(news.size(), 3);

    newsService->setNewsPerDay(0);
    ASSERT_NE(newsService->getNewsPerDay(), 0);

    newsService->setNewsPerDay(-2);
    ASSERT_NE(newsService->getNewsPerDay(), -2);
}