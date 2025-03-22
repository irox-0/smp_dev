#include <gtest/gtest.h>
#include "services/SaveService.hpp"
#include "core/Market.hpp"
#include "core/Player.hpp"
#include "services/NewsService.hpp"
#include "services/PriceService.hpp"
#include "utils/Random.hpp"

using namespace StockMarketSimulator;

class SaveServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        Random::initialize(42);
        
        market = std::make_shared<Market>();
        market->addDefaultCompanies();
        
        player = std::make_shared<Player>("Test Player", 10000.0);
        player->setMarket(market);
        
        newsService = std::make_shared<NewsService>(market);
        newsService->initialize();
        
        priceService = std::make_shared<PriceService>(market);
        priceService->initialize();
        
        saveService = std::make_unique<SaveService>(market, player, newsService, priceService);
        saveService->initialize(testSavesDir);
        
        if (!FileIO::directoryExists(testSavesDir)) {
            FileIO::createDirectory(testSavesDir);
        }
    }
    
    void TearDown() override {
        auto saveFiles = saveService->listSaves();
        for (const auto& save : saveFiles) {
            saveService->deleteSave(save.filename);
        }
    }
    
    std::shared_ptr<Market> market;
    std::shared_ptr<Player> player;
    std::shared_ptr<NewsService> newsService;
    std::shared_ptr<PriceService> priceService;
    std::unique_ptr<SaveService> saveService;
    
    const std::string testSavesDir = "test_saves";
};

TEST_F(SaveServiceTest, InitializationTest) {
    ASSERT_EQ(saveService->getSavesDirectory(), testSavesDir);
    ASSERT_TRUE(saveService->isAutosaveEnabled());
    ASSERT_EQ(saveService->getAutosaveInterval(), 5);
}

TEST_F(SaveServiceTest, SaveLoadGameTest) {
    player->buyStock(market->getCompanyByTicker("TCH"), 20);
    player->setCurrentDay(5);
    
    double initialBalance = player->getPortfolio()->getCashBalance();
    int initialDay = player->getCurrentDay();
    
    bool saveResult = saveService->saveGame("Test Save");
    ASSERT_TRUE(saveResult);
    
    auto saves = saveService->listSaves();
    ASSERT_EQ(saves.size(), 1);
    
    std::string saveFilename = saves[0].filename;
    ASSERT_FALSE(saveFilename.empty());
    
    player->sellStock(market->getCompanyByTicker("TCH"), 10);
    player->setCurrentDay(10);
    
    bool loadResult = saveService->loadGame(saveFilename);
    ASSERT_TRUE(loadResult);
    
    ASSERT_NEAR(player->getPortfolio()->getCashBalance(), initialBalance, 0.01);
    ASSERT_EQ(player->getCurrentDay(), initialDay);
    ASSERT_EQ(player->getPortfolio()->getPositionQuantity("TCH"), 20);
}

TEST_F(SaveServiceTest, SaveMetadataTest) {
    player->setCurrentDay(7);
    double playerNetWorth = player->getNetWorth();
    
    saveService->saveGame("Metadata Test");
    
    auto saves = saveService->listSaves();
    ASSERT_EQ(saves.size(), 1);
    
    SaveMetadata metadata = saves[0];
    ASSERT_EQ(metadata.displayName, "Metadata Test");
    ASSERT_EQ(metadata.gameDay, 7);
    ASSERT_NEAR(metadata.playerNetWorth, playerNetWorth, 0.01);
    ASSERT_FALSE(metadata.isAutosave);
    
    SaveMetadata individualMetadata = saveService->getSaveMetadata(metadata.filename);
    ASSERT_EQ(individualMetadata.displayName, metadata.displayName);
    ASSERT_EQ(individualMetadata.gameDay, metadata.gameDay);
    ASSERT_NEAR(individualMetadata.playerNetWorth, metadata.playerNetWorth, 0.01);
}

TEST_F(SaveServiceTest, AutosaveTest) {
    player->setCurrentDay(0);
    saveService->setAutosave(true, 3);
    
    ASSERT_FALSE(saveService->checkAndCreateAutosave());
    
    player->setCurrentDay(3);
    
    ASSERT_TRUE(saveService->checkAndCreateAutosave());
    
    auto saves = saveService->listSaves();
    ASSERT_EQ(saves.size(), 1);
    
    player->setCurrentDay(4);
    ASSERT_FALSE(saveService->checkAndCreateAutosave());
    
    player->setCurrentDay(6);
    ASSERT_TRUE(saveService->checkAndCreateAutosave());
    
    saves = saveService->listSaves();
    ASSERT_EQ(saves.size(), 2);
    
    saveService->setAutosave(false);
    player->setCurrentDay(9);
    ASSERT_FALSE(saveService->checkAndCreateAutosave());
}

TEST_F(SaveServiceTest, DeleteSaveTest) {
    saveService->saveGame("Save 1");
    saveService->saveGame("Save 2");
    
    auto saves = saveService->listSaves();
    ASSERT_EQ(saves.size(), 2);
    
    bool deleteResult = saveService->deleteSave(saves[0].filename);
    ASSERT_TRUE(deleteResult);
    
    saves = saveService->listSaves();
    ASSERT_EQ(saves.size(), 1);
    
    ASSERT_FALSE(saveService->deleteSave("nonexistent_save.json"));
}

TEST_F(SaveServiceTest, InvalidSaveTest) {
    ASSERT_FALSE(saveService->loadGame("nonexistent_save.json"));
    
    nlohmann::json invalidData;
    invalidData["invalid"] = "data";
    
    std::string invalidPath = FileIO::combineFilePath(testSavesDir, "invalid_save.json");
    FileIO::writeJsonFile(invalidPath, invalidData);
    
    ASSERT_FALSE(saveService->loadGame("invalid_save.json"));
}

TEST_F(SaveServiceTest, WeakPtrTest) {
    SaveService emptyService;
    ASSERT_FALSE(emptyService.saveGame("Empty Save"));
    ASSERT_FALSE(emptyService.loadGame("nonexistent.json"));
    ASSERT_FALSE(emptyService.checkAndCreateAutosave());
    
    SaveService marketOnlyService(market, std::weak_ptr<Player>(), std::weak_ptr<NewsService>(), std::weak_ptr<PriceService>());
    marketOnlyService.initialize(testSavesDir);
    ASSERT_FALSE(marketOnlyService.saveGame("Market Only Save"));
    
    SaveService playerOnlyService(std::weak_ptr<Market>(), player, std::weak_ptr<NewsService>(), std::weak_ptr<PriceService>());
    playerOnlyService.initialize(testSavesDir);
    ASSERT_FALSE(playerOnlyService.saveGame("Player Only Save"));
}

TEST_F(SaveServiceTest, SettersTest) {
    SaveService service;
    
    service.setMarket(market);
    service.setPlayer(player);
    service.setNewsService(newsService);
    service.setPriceService(priceService);
    
    service.setSavesDirectory("custom_saves");
    ASSERT_EQ(service.getSavesDirectory(), "custom_saves");
    
    service.setAutosave(false);
    ASSERT_FALSE(service.isAutosaveEnabled());
    
    service.setAutosave(true, 10);
    ASSERT_TRUE(service.isAutosaveEnabled());
    ASSERT_EQ(service.getAutosaveInterval(), 10);
    
    service.setAutosave(true, -5);
    ASSERT_EQ(service.getAutosaveInterval(), 10);
}

TEST_F(SaveServiceTest, ComplexGameStateTest) {
    player->buyStock(market->getCompanyByTicker("TCH"), 20);
    player->buyStock(market->getCompanyByTicker("EPLC"), 30);
    player->takeLoan(5000.0, 0.05, 30, "Test Loan");
    player->depositToMarginAccount(2000.0);
    player->setCurrentDay(10);
    
    newsService->setCurrentDay(10);
    auto news = newsService->generateDailyNews(3);
    newsService->applyNewsEffects(news);
    
    priceService->updatePrices();
    
    bool saveResult = saveService->saveGame("Complex State");
    ASSERT_TRUE(saveResult);
    
    player->setCurrentDay(20);
    player->sellStock(market->getCompanyByTicker("TCH"), 10);
    
    auto saves = saveService->listSaves();
    bool loadResult = saveService->loadGame(saves[0].filename);
    ASSERT_TRUE(loadResult);
    
    ASSERT_EQ(player->getCurrentDay(), 10);
    ASSERT_EQ(player->getPortfolio()->getPositionQuantity("TCH"), 20);
    ASSERT_EQ(player->getPortfolio()->getPositionQuantity("EPLC"), 30);
    ASSERT_GT(player->getLoans().size(), 0);
    ASSERT_GT(player->getMarginAccountBalance(), 0.0);
}