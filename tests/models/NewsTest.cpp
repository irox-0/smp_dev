#include <gtest/gtest.h>
#include "models/News.hpp"
#include "models/Company.hpp"

using namespace StockMarketSimulator;

class NewsTest : public ::testing::Test {
protected:
    void SetUp() override {
        techCompany = std::make_shared<Company>(
            "Tech Corp", "TECH",
            "Technology company", Sector::Technology,
            100.0, 0.5, DividendPolicy(2.0, 4)
        );
        
        energyCompany = std::make_shared<Company>(
            "Energy Corp", "ENRG",
            "Energy company", Sector::Energy,
            50.0, 0.4, DividendPolicy(3.0, 4)
        );
        
        globalNews = std::make_unique<News>(
            NewsType::Global,
            "Central Bank Reduces Interest Rate",
            "Central Bank has announced a 0.5% reduction in the base interest rate.",
            0.02, 1
        );
        
        sectorNews = std::make_unique<News>(
            NewsType::Sector,
            "New Regulations in Technology Sector",
            "Government introduces new regulations affecting technology companies.",
            -0.01, 1, Sector::Technology
        );
        
        corporateNews = std::make_unique<News>(
            NewsType::Corporate,
            "Tech Corp Announces New Product",
            "Tech Corp has announced a revolutionary new product that will hit the market next quarter.",
            0.05, 1, techCompany
        );
    }
    
    std::shared_ptr<Company> techCompany;
    std::shared_ptr<Company> energyCompany;
    std::unique_ptr<News> globalNews;
    std::unique_ptr<News> sectorNews;
    std::unique_ptr<News> corporateNews;
    
    NewsTemplate createTestTemplate() {
        return NewsTemplate(
            NewsType::Global,
            "Economy %s by %d%%",
            "The economy has %s by %d%% according to latest reports.",
            -0.03, 0.03,
            true, false
        );
    }
};

TEST_F(NewsTest, InitializationTest) {
    ASSERT_EQ(globalNews->getType(), NewsType::Global);
    ASSERT_EQ(globalNews->getTitle(), "Central Bank Reduces Interest Rate");
    ASSERT_EQ(globalNews->getContent(), "Central Bank has announced a 0.5% reduction in the base interest rate.");
    ASSERT_DOUBLE_EQ(globalNews->getImpact(), 0.02);
    ASSERT_EQ(globalNews->getPublishDay(), 1);
    ASSERT_FALSE(globalNews->isProcessed());
    
    ASSERT_EQ(sectorNews->getType(), NewsType::Sector);
    ASSERT_EQ(sectorNews->getTargetSector(), Sector::Technology);
    
    ASSERT_EQ(corporateNews->getType(), NewsType::Corporate);
    auto company = corporateNews->getTargetCompany().lock();
    ASSERT_NE(company, nullptr);
    ASSERT_EQ(company->getTicker(), "TECH");
}

TEST_F(NewsTest, SettersTest) {
    News news;
    
    news.setType(NewsType::Global);
    ASSERT_EQ(news.getType(), NewsType::Global);
    
    news.setTitle("Test Title");
    ASSERT_EQ(news.getTitle(), "Test Title");
    
    news.setContent("Test Content");
    ASSERT_EQ(news.getContent(), "Test Content");
    
    news.setImpact(0.03);
    ASSERT_DOUBLE_EQ(news.getImpact(), 0.03);
    
    news.setPublishDay(5);
    ASSERT_EQ(news.getPublishDay(), 5);
    
    news.setTargetSector(Sector::Energy);
    ASSERT_EQ(news.getTargetSector(), Sector::Energy);
    
    news.setTargetCompany(techCompany);
    auto company = news.getTargetCompany().lock();
    ASSERT_NE(company, nullptr);
    ASSERT_EQ(company->getTicker(), "TECH");
    
    news.setProcessed(true);
    ASSERT_TRUE(news.isProcessed());
}

TEST_F(NewsTest, AffectMethodsTest) {
    ASSERT_TRUE(globalNews->shouldAffectMarket());
    ASSERT_TRUE(globalNews->shouldAffectSector(Sector::Technology));
    ASSERT_TRUE(globalNews->shouldAffectSector(Sector::Energy));
    ASSERT_TRUE(globalNews->shouldAffectCompany(techCompany));
    ASSERT_TRUE(globalNews->shouldAffectCompany(energyCompany));

    ASSERT_FALSE(sectorNews->shouldAffectMarket());
    ASSERT_TRUE(sectorNews->shouldAffectSector(Sector::Technology));
    ASSERT_FALSE(sectorNews->shouldAffectSector(Sector::Energy));
    ASSERT_TRUE(sectorNews->shouldAffectCompany(techCompany));
    ASSERT_FALSE(sectorNews->shouldAffectCompany(energyCompany));

    ASSERT_FALSE(corporateNews->shouldAffectMarket());
    ASSERT_FALSE(corporateNews->shouldAffectSector(Sector::Energy));
    ASSERT_TRUE(corporateNews->shouldAffectCompany(techCompany));
    ASSERT_FALSE(corporateNews->shouldAffectCompany(energyCompany));
}
TEST_F(NewsTest, TypeConversionTest) {
    ASSERT_EQ(News::newsTypeToString(NewsType::Global), "Global");
    ASSERT_EQ(News::newsTypeToString(NewsType::Sector), "Sector");
    ASSERT_EQ(News::newsTypeToString(NewsType::Corporate), "Corporate");
    
    ASSERT_EQ(News::newsTypeFromString("Global"), NewsType::Global);
    ASSERT_EQ(News::newsTypeFromString("Sector"), NewsType::Sector);
    ASSERT_EQ(News::newsTypeFromString("Corporate"), NewsType::Corporate);
    ASSERT_EQ(News::newsTypeFromString("Unknown"), NewsType::Global);
}

TEST_F(NewsTest, JsonSerializationTest) {
    nlohmann::json json = globalNews->toJson();
    
    ASSERT_EQ(json["type"], "Global");
    ASSERT_EQ(json["title"], "Central Bank Reduces Interest Rate");
    ASSERT_EQ(json["content"], "Central Bank has announced a 0.5% reduction in the base interest rate.");
    ASSERT_EQ(json["impact"], 0.02);
    ASSERT_EQ(json["publish_day"], 1);
    ASSERT_EQ(json["processed"], false);
    
    json = corporateNews->toJson();
    ASSERT_EQ(json["type"], "Corporate");
    ASSERT_EQ(json["target_company_ticker"], "TECH");
    
    std::vector<std::shared_ptr<Company>> companies = {techCompany, energyCompany};
    News restoredNews = News::fromJson(json, companies);
    
    ASSERT_EQ(restoredNews.getType(), NewsType::Corporate);
    ASSERT_EQ(restoredNews.getTitle(), corporateNews->getTitle());
    ASSERT_EQ(restoredNews.getContent(), corporateNews->getContent());
    ASSERT_DOUBLE_EQ(restoredNews.getImpact(), corporateNews->getImpact());
    
    auto company = restoredNews.getTargetCompany().lock();
    ASSERT_NE(company, nullptr);
    ASSERT_EQ(company->getTicker(), "TECH");
}

TEST_F(NewsTest, NewsTemplateTest) {
    NewsTemplate templ = createTestTemplate();
    
    ASSERT_EQ(templ.type, NewsType::Global);
    ASSERT_EQ(templ.titleTemplate, "Economy %s by %d%%");
    ASSERT_EQ(templ.contentTemplate, "The economy has %s by %d%% according to latest reports.");
    ASSERT_DOUBLE_EQ(templ.minImpact, -0.03);
    ASSERT_DOUBLE_EQ(templ.maxImpact, 0.03);
    ASSERT_TRUE(templ.requiresPositiveMarket);
    ASSERT_FALSE(templ.requiresNegativeMarket);
    
    nlohmann::json json = templ.toJson();
    
    ASSERT_EQ(json["type"], "Global");
    ASSERT_EQ(json["title_template"], "Economy %s by %d%%");
    ASSERT_EQ(json["content_template"], "The economy has %s by %d%% according to latest reports.");
    ASSERT_EQ(json["min_impact"], -0.03);
    ASSERT_EQ(json["max_impact"], 0.03);
    ASSERT_EQ(json["requires_positive_market"], true);
    ASSERT_EQ(json["requires_negative_market"], false);
    
    NewsTemplate restoredTemplate = NewsTemplate::fromJson(json);
    
    ASSERT_EQ(restoredTemplate.type, templ.type);
    ASSERT_EQ(restoredTemplate.titleTemplate, templ.titleTemplate);
    ASSERT_EQ(restoredTemplate.contentTemplate, templ.contentTemplate);
    ASSERT_DOUBLE_EQ(restoredTemplate.minImpact, templ.minImpact);
    ASSERT_DOUBLE_EQ(restoredTemplate.maxImpact, templ.maxImpact);
    ASSERT_EQ(restoredTemplate.requiresPositiveMarket, templ.requiresPositiveMarket);
    ASSERT_EQ(restoredTemplate.requiresNegativeMarket, templ.requiresNegativeMarket);
}

TEST_F(NewsTest, EdgeCasesTest) {
    News emptyNews;
    ASSERT_EQ(emptyNews.getType(), NewsType::Global);
    ASSERT_EQ(emptyNews.getTitle(), "");
    ASSERT_EQ(emptyNews.getContent(), "");
    ASSERT_DOUBLE_EQ(emptyNews.getImpact(), 0.0);
    
    emptyNews.setPublishDay(-5);
    ASSERT_EQ(emptyNews.getPublishDay(), 0);
    
    std::weak_ptr<Company> nullCompany;
    News newsWithNullCompany(NewsType::Corporate, "Test", "Test", 0.0, 1, nullCompany);
    ASSERT_EQ(newsWithNullCompany.getTargetCompany().lock(), nullptr);
    
    News newsFromJson = News::fromJson(corporateNews->toJson());
    ASSERT_EQ(newsFromJson.getTargetCompany().lock(), nullptr);
}