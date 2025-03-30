#include <gtest/gtest.h>
#include <memory>
#include <string>
#include "../../src/services/SaveService.hpp"
#include "../../src/core/Market.hpp"
#include "../../src/core/Player.hpp"
#include "../../src/services/NewsService.hpp"
#include "../../src/services/PriceService.hpp"
#include "../../src/utils/FileIO.hpp"

using namespace StockMarketSimulator;

// Test fixture for SaveService tests
class SaveServiceTest : public ::testing::Test {
protected:
    std::shared_ptr<Market> market;
    std::shared_ptr<Player> player;
    std::shared_ptr<NewsService> newsService;
    std::shared_ptr<PriceService> priceService;
    std::shared_ptr<SaveService> saveService;
    std::string testSavesDirectory;

    void SetUp() override {
        // Create test directory
        testSavesDirectory = "test_saves";
        if (!FileIO::directoryExists(testSavesDirectory)) {
            FileIO::createDirectory(testSavesDirectory);
        } else {
            // Clean up any existing test saves
            std::vector<std::string> saveFiles = FileIO::listFiles(testSavesDirectory);
            for (const auto& file : saveFiles) {
                std::string filePath = FileIO::combineFilePath(testSavesDirectory, file);
                std::remove(filePath.c_str());
            }
        }

        // Initialize market and player
        market = std::make_shared<Market>();
        market->addDefaultCompanies();

        player = std::make_shared<Player>("TestPlayer", 10000.0);
        player->setMarket(market);

        newsService = std::make_shared<NewsService>(market);
        newsService->initialize();

        priceService = std::make_shared<PriceService>(market);
        priceService->initialize();

        // Initialize save service
        saveService = std::make_shared<SaveService>(market, player, newsService, priceService);
        saveService->initialize(testSavesDirectory);
    }

    void TearDown() override {
        // Clean up test directory
        if (FileIO::directoryExists(testSavesDirectory)) {
            std::vector<std::string> saveFiles = FileIO::listFiles(testSavesDirectory);
            for (const auto& file : saveFiles) {
                std::string filePath = FileIO::combineFilePath(testSavesDirectory, file);
                std::remove(filePath.c_str());
            }
            // Use rmdir to remove the directory instead of std::filesystem
            // This avoids potential DLL dependencies
            #ifdef _WIN32
            _rmdir(testSavesDirectory.c_str());
            #else
            rmdir(testSavesDirectory.c_str());
            #endif
        }
    }

    // Helper method to simulate game progression
    void progressGame(int days) {
        for (int i = 0; i < days; i++) {
            market->simulateDay();
            player->updateDailyState();
            player->closeDay();
        }
    }
};

// Test SaveMetadata with Date
TEST_F(SaveServiceTest, SaveMetadataDate) {
    // Create metadata with a specific date
    Date testDate(15, 4, 2023);
    SaveMetadata metadata("test.json", "Test Save", testDate, 12345.67, "2023-04-15 12:00:00", false);

    // Convert to JSON and back
    nlohmann::json j = metadata.toJson();
    SaveMetadata restored = SaveMetadata::fromJson(j);

    // Verify date is properly serialized and deserialized
    EXPECT_EQ(restored.gameDate.getDay(), 15);
    EXPECT_EQ(restored.gameDate.getMonth(), 4);
    EXPECT_EQ(restored.gameDate.getYear(), 2023);
    EXPECT_EQ(restored.displayName, "Test Save");
    EXPECT_DOUBLE_EQ(restored.playerNetWorth, 12345.67);
}

// Test saving and loading game with Date
TEST_F(SaveServiceTest, SaveAndLoadGame) {
    // Progress game a few days to have interesting data
    progressGame(10);

    // Remember current date and player net worth
    Date originalDate = player->getCurrentDate();
    double originalNetWorth = player->getNetWorth();

    // Save the game
    std::string saveName = "TestSave";
    bool saveResult = saveService->saveGame(saveName);
    EXPECT_TRUE(saveResult);

    // Get the save files
    std::vector<SaveMetadata> saves = saveService->listSaves();
    EXPECT_FALSE(saves.empty());

    // Reset player and market data
    player = std::make_shared<Player>("NewPlayer", 5000.0);
    market = std::make_shared<Market>();
    player->setMarket(market);

    // Update the components in the save service
    saveService->setPlayer(player);
    saveService->setMarket(market);

    // Load the game
    bool loadResult = saveService->loadGame(saves[0].filename);
    EXPECT_TRUE(loadResult);

    // Verify the data was loaded correctly
    EXPECT_EQ(player->getName(), "TestPlayer");
    EXPECT_NEAR(player->getNetWorth(), originalNetWorth, 0.01);

    // Verify the date was loaded correctly
    EXPECT_EQ(player->getCurrentDate().getDay(), originalDate.getDay());
    EXPECT_EQ(player->getCurrentDate().getMonth(), originalDate.getMonth());
    EXPECT_EQ(player->getCurrentDate().getYear(), originalDate.getYear());
}

// Test autosave functionality with Date
TEST_F(SaveServiceTest, AutosaveWithDate) {
    // Enable autosave with interval of 5 days
    saveService->setAutosave(true, 5);
    EXPECT_TRUE(saveService->isAutosaveEnabled());
    EXPECT_EQ(saveService->getAutosaveInterval(), 5);

    // Progress game 4 days - no autosave should happen
    progressGame(4);
    bool autosaveResult = saveService->checkAndCreateAutosave();
    EXPECT_FALSE(autosaveResult);

    // Get saves - should be empty
    std::vector<SaveMetadata> saves = saveService->listSaves();
    EXPECT_TRUE(saves.empty());

    // Progress one more day - autosave should happen
    progressGame(1);
    autosaveResult = saveService->checkAndCreateAutosave();
    EXPECT_TRUE(autosaveResult);

    // Get saves - should have one autosave
    saves = saveService->listSaves();
    EXPECT_EQ(saves.size(), 1);
    EXPECT_TRUE(saves[0].isAutosave);

    // Verify the date in the autosave matches player's date
    Date playerDate = player->getCurrentDate();
    EXPECT_EQ(saves[0].gameDate.getDay(), playerDate.getDay());
    EXPECT_EQ(saves[0].gameDate.getMonth(), playerDate.getMonth());
    EXPECT_EQ(saves[0].gameDate.getYear(), playerDate.getYear());
}

// Test save filename generation with Date
TEST_F(SaveServiceTest, SaveFilenameGeneration) {
    // Set player date to a specific value
    Date specificDate(25, 12, 2023);
    player->setCurrentDate(specificDate);

    // Save game
    bool saveResult = saveService->saveGame("Christmas Save");
    EXPECT_TRUE(saveResult);

    // Get the save files
    std::vector<SaveMetadata> saves = saveService->listSaves();
    EXPECT_EQ(saves.size(), 1);

    // Verify the filename contains the date
    EXPECT_NE(saves[0].filename.find("25.12.2023"), std::string::npos);
    EXPECT_NE(saves[0].filename.find("Christmas_Save"), std::string::npos);
}

// Test save metadata retrieval
TEST_F(SaveServiceTest, GetSaveMetadata) {
    // Progress game and save
    progressGame(15);
    bool saveResult = saveService->saveGame("Test Metadata");
    EXPECT_TRUE(saveResult);

    // Get saves
    std::vector<SaveMetadata> saves = saveService->listSaves();
    EXPECT_EQ(saves.size(), 1);

    // Get metadata for the save
    SaveMetadata metadata = saveService->getSaveMetadata(saves[0].filename);

    // Verify the metadata
    EXPECT_EQ(metadata.displayName, "Test Metadata");
    EXPECT_NEAR(metadata.playerNetWorth, player->getNetWorth(), 0.01);

    // Verify the date matches player's date
    Date playerDate = player->getCurrentDate();
    EXPECT_EQ(metadata.gameDate.getDay(), playerDate.getDay());
    EXPECT_EQ(metadata.gameDate.getMonth(), playerDate.getMonth());
    EXPECT_EQ(metadata.gameDate.getYear(), playerDate.getYear());
}

// Test save deletion
TEST_F(SaveServiceTest, DeleteSave) {
    // Save game
    bool saveResult = saveService->saveGame("Doomed Save");
    EXPECT_TRUE(saveResult);

    // Verify save exists
    std::vector<SaveMetadata> saves = saveService->listSaves();
    EXPECT_EQ(saves.size(), 1);

    // Delete the save
    bool deleteResult = saveService->deleteSave(saves[0].filename);
    EXPECT_TRUE(deleteResult);

    // Verify save no longer exists
    saves = saveService->listSaves();
    EXPECT_TRUE(saves.empty());
}

// Test saving and loading with all components
TEST_F(SaveServiceTest, SaveAndLoadWithAllComponents) {
    // Progress the game and create some news
    progressGame(5);
    auto news = newsService->generateDailyNews(3);
    newsService->applyNewsEffects(news);

    // Save the game
    bool saveResult = saveService->saveGame("Full Component Test");
    EXPECT_TRUE(saveResult);

    // Create new instances of all components
    auto newMarket = std::make_shared<Market>();
    auto newPlayer = std::make_shared<Player>("NewPlayer", 5000.0);
    auto newNewsService = std::make_shared<NewsService>();
    auto newPriceService = std::make_shared<PriceService>();

    newPlayer->setMarket(newMarket);
    newNewsService->setMarket(newMarket);
    newPriceService->setMarket(newMarket);

    // Create new save service with new components
    auto newSaveService = std::make_shared<SaveService>(
        newMarket, newPlayer, newNewsService, newPriceService);
    newSaveService->initialize(testSavesDirectory);

    // Get the saves
    std::vector<SaveMetadata> saves = newSaveService->listSaves();
    EXPECT_EQ(saves.size(), 1);

    // Load the save
    bool loadResult = newSaveService->loadGame(saves[0].filename);
    EXPECT_TRUE(loadResult);

    // Verify market data loaded correctly
    EXPECT_EQ(newMarket->getCompanies().size(), market->getCompanies().size());

    // Verify player data loaded correctly
    EXPECT_EQ(newPlayer->getName(), "TestPlayer");
    EXPECT_NEAR(newPlayer->getNetWorth(), player->getNetWorth(), 0.01);

    // Verify date loaded correctly
    Date originalDate = player->getCurrentDate();
    Date loadedDate = newPlayer->getCurrentDate();
    EXPECT_EQ(loadedDate.getDay(), originalDate.getDay());
    EXPECT_EQ(loadedDate.getMonth(), originalDate.getMonth());
    EXPECT_EQ(loadedDate.getYear(), originalDate.getYear());

    // Verify news service data loaded correctly
    EXPECT_FALSE(newNewsService->getNewsHistory().empty());
}