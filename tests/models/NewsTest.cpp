#include <gtest/gtest.h>
#include <memory>
#include "../../src/models/News.hpp"
#include "../../src/models/Company.hpp"
#include "../../src/utils/Date.hpp"

using namespace StockMarketSimulator;

class NewsTest : public ::testing::Test {
protected:
    std::shared_ptr<Company> testCompany;
    std::shared_ptr<Company> anotherCompany;
    Date testDate;

    void SetUp() override {
        // Create a test company
        testCompany = std::make_shared<Company>(
            "TestCorp", "TEST",
            "A test company for unit tests",
            Sector::Technology,
            100.0, 0.5,
            DividendPolicy(1.0, 4)
        );

        // Create another company for comparison tests
        anotherCompany = std::make_shared<Company>(
            "AnotherCorp", "ANTH",
            "Another test company",
            Sector::Energy,
            50.0, 0.3,
            DividendPolicy(0.5, 2)
        );

        // Create a test date (April 15, 2023)
        testDate = Date(15, 4, 2023);
    }
};

// Test the basic constructor and getters
TEST_F(NewsTest, ConstructorAndGetters) {
    // Create a news item
    News news(NewsType::Global, "Test Title", "Test Content", 0.02, testDate);

    // Test getters
    EXPECT_EQ(news.getType(), NewsType::Global);
    EXPECT_EQ(news.getTitle(), "Test Title");
    EXPECT_EQ(news.getContent(), "Test Content");
    EXPECT_DOUBLE_EQ(news.getImpact(), 0.02);
    EXPECT_EQ(news.getPublishDate(), testDate);
    EXPECT_EQ(news.getTargetSector(), Sector::Unknown);
    EXPECT_FALSE(news.isProcessed());
}

// Test news with sector target
TEST_F(NewsTest, SectorTargetNews) {
    // Create a sector news item
    News news(NewsType::Sector, "Sector News", "Sector Content", -0.01, testDate, Sector::Technology);

    // Test sector-specific getters
    EXPECT_EQ(news.getType(), NewsType::Sector);
    EXPECT_EQ(news.getTargetSector(), Sector::Technology);

    // Test shouldAffectMarket and shouldAffectSector methods
    EXPECT_FALSE(news.shouldAffectMarket());
    EXPECT_TRUE(news.shouldAffectSector(Sector::Technology));
    EXPECT_FALSE(news.shouldAffectSector(Sector::Energy));

    // Test shouldAffectCompany method
    EXPECT_TRUE(news.shouldAffectCompany(testCompany));       // Technology company
    EXPECT_FALSE(news.shouldAffectCompany(anotherCompany));   // Energy company
}

// Test news with company target
TEST_F(NewsTest, CompanyTargetNews) {
    // Create a company news item
    News news(NewsType::Corporate, "Company News", "Company Content", 0.03, testDate, testCompany);

    // Test company-specific getters
    EXPECT_EQ(news.getType(), NewsType::Corporate);
    EXPECT_EQ(news.getTargetSector(), Sector::Technology); // Should be set from testCompany's sector

    auto targetCompany = news.getTargetCompany().lock();
    ASSERT_TRUE(targetCompany);
    EXPECT_EQ(targetCompany->getTicker(), "TEST");

    // Test shouldAffectMarket, shouldAffectSector, and shouldAffectCompany methods
    EXPECT_FALSE(news.shouldAffectMarket());
    EXPECT_FALSE(news.shouldAffectCompany(anotherCompany));
    EXPECT_TRUE(news.shouldAffectCompany(testCompany));
}

// Test global news affecting everything
TEST_F(NewsTest, GlobalNewsEffects) {
    // Create global news
    News news(NewsType::Global, "Global News", "Global Content", 0.05, testDate);

    // Global news should affect everything
    EXPECT_TRUE(news.shouldAffectMarket());
    EXPECT_TRUE(news.shouldAffectSector(Sector::Technology));
    EXPECT_TRUE(news.shouldAffectSector(Sector::Energy));
    EXPECT_TRUE(news.shouldAffectCompany(testCompany));
    EXPECT_TRUE(news.shouldAffectCompany(anotherCompany));
}

// Test setters
TEST_F(NewsTest, Setters) {
    News news;

    // Test setters
    news.setType(NewsType::Sector);
    news.setTitle("New Title");
    news.setContent("New Content");
    news.setImpact(-0.03);
    news.setPublishDate(testDate);
    news.setTargetSector(Sector::Finance);
    news.setProcessed(true);

    // Check if setters worked
    EXPECT_EQ(news.getType(), NewsType::Sector);
    EXPECT_EQ(news.getTitle(), "New Title");
    EXPECT_EQ(news.getContent(), "New Content");
    EXPECT_DOUBLE_EQ(news.getImpact(), -0.03);
    EXPECT_EQ(news.getPublishDate(), testDate);
    EXPECT_EQ(news.getTargetSector(), Sector::Finance);
    EXPECT_TRUE(news.isProcessed());

    // Test setTargetCompany
    news.setTargetCompany(testCompany);
    auto company = news.getTargetCompany().lock();
    ASSERT_TRUE(company);
    EXPECT_EQ(company->getTicker(), "TEST");

    // Target sector should now be Technology (from testCompany)
    EXPECT_EQ(news.getTargetSector(), Sector::Technology);
}

// Test JSON serialization and deserialization
TEST_F(NewsTest, JsonSerialization) {
    // Create a complete news item
    News original(NewsType::Corporate, "JSON Test", "Testing JSON serialization", 0.04, testDate, testCompany);

    // Serialize to JSON
    nlohmann::json json = original.toJson();

    // Check JSON content
    EXPECT_EQ(json["type"], "Corporate");
    EXPECT_EQ(json["title"], "JSON Test");
    EXPECT_EQ(json["content"], "Testing JSON serialization");
    EXPECT_DOUBLE_EQ(json["impact"], 0.04);
    EXPECT_FALSE(json["processed"]);
    EXPECT_EQ(json["target_sector"], "Technology");
    EXPECT_EQ(json["target_company_ticker"], "TEST");

    // Check publish_date JSON
    ASSERT_TRUE(json.contains("publish_date"));
    EXPECT_EQ(json["publish_date"]["day"], 15);
    EXPECT_EQ(json["publish_date"]["month"], 4);
    EXPECT_EQ(json["publish_date"]["year"], 2023);

    // Create a vector of companies for deserialization
    std::vector<std::shared_ptr<Company>> companies = {testCompany, anotherCompany};

    // Deserialize from JSON
    News deserialized = News::fromJson(json, companies);

    // Check if deserialized news matches original
    EXPECT_EQ(deserialized.getType(), original.getType());
    EXPECT_EQ(deserialized.getTitle(), original.getTitle());
    EXPECT_EQ(deserialized.getContent(), original.getContent());
    EXPECT_DOUBLE_EQ(deserialized.getImpact(), original.getImpact());
    EXPECT_EQ(deserialized.getPublishDate(), original.getPublishDate());
    EXPECT_EQ(deserialized.getTargetSector(), original.getTargetSector());
    EXPECT_EQ(deserialized.isProcessed(), original.isProcessed());

    // Check if company reference is properly restored
    auto deserializedCompany = deserialized.getTargetCompany().lock();
    ASSERT_TRUE(deserializedCompany);
    EXPECT_EQ(deserializedCompany->getTicker(), "TEST");
}

// Test backward compatibility with old JSON format (using publish_day)
TEST_F(NewsTest, BackwardCompatibilityJson) {
    // Create JSON with old format (publish_day as int)
    nlohmann::json oldJson = {
        {"type", "Global"},
        {"title", "Old Format News"},
        {"content", "This news uses the old format with publish_day"},
        {"impact", 0.01},
        {"publish_day", 45},  // Assuming day 45 from reference date
        {"processed", false},
        {"target_sector", "Unknown"},
        {"target_company_ticker", ""}
    };

    // Deserialize from old JSON format
    News deserialized = News::fromJson(oldJson);

    // Check if deserialization worked
    EXPECT_EQ(deserialized.getType(), NewsType::Global);
    EXPECT_EQ(deserialized.getTitle(), "Old Format News");

    // The publish_day should have been converted to a Date
    // Day 45 from March 1, 2023 should be around April 15, 2023
    Date expectedDate = Date::fromDayNumber(45);
    EXPECT_EQ(deserialized.getPublishDate(), expectedDate);
}

// Test NewsTemplate functionality
TEST_F(NewsTest, NewsTemplate) {
    // Create a news template
    NewsTemplate tmpl(
        NewsType::Sector,
        "Template Title for %s",
        "Template Content about %s with number %d",
        -0.02, 0.02,
        true, false,
        Sector::Finance
    );

    // Check template properties
    EXPECT_EQ(tmpl.type, NewsType::Sector);
    EXPECT_EQ(tmpl.titleTemplate, "Template Title for %s");
    EXPECT_EQ(tmpl.contentTemplate, "Template Content about %s with number %d");
    EXPECT_DOUBLE_EQ(tmpl.minImpact, -0.02);
    EXPECT_DOUBLE_EQ(tmpl.maxImpact, 0.02);
    EXPECT_TRUE(tmpl.requiresPositiveMarket);
    EXPECT_FALSE(tmpl.requiresNegativeMarket);
    EXPECT_EQ(tmpl.targetSector, Sector::Finance);

    // Test NewsTemplate serialization
    nlohmann::json json = tmpl.toJson();

    // Check JSON content
    EXPECT_EQ(json["type"], "Sector");
    EXPECT_EQ(json["title_template"], "Template Title for %s");
    EXPECT_EQ(json["content_template"], "Template Content about %s with number %d");
    EXPECT_DOUBLE_EQ(json["min_impact"], -0.02);
    EXPECT_DOUBLE_EQ(json["max_impact"], 0.02);
    EXPECT_TRUE(json["requires_positive_market"]);
    EXPECT_FALSE(json["requires_negative_market"]);
    EXPECT_EQ(json["target_sector"], "Finance");

    // Test NewsTemplate deserialization
    NewsTemplate deserialized = NewsTemplate::fromJson(json);

    // Check if deserialized template matches original
    EXPECT_EQ(deserialized.type, tmpl.type);
    EXPECT_EQ(deserialized.titleTemplate, tmpl.titleTemplate);
    EXPECT_EQ(deserialized.contentTemplate, tmpl.contentTemplate);
    EXPECT_DOUBLE_EQ(deserialized.minImpact, tmpl.minImpact);
    EXPECT_DOUBLE_EQ(deserialized.maxImpact, tmpl.maxImpact);
    EXPECT_EQ(deserialized.requiresPositiveMarket, tmpl.requiresPositiveMarket);
    EXPECT_EQ(deserialized.requiresNegativeMarket, tmpl.requiresNegativeMarket);
    EXPECT_EQ(deserialized.targetSector, tmpl.targetSector);
}
