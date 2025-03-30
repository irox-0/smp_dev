#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "../core/Market.hpp"
#include "../core/Player.hpp"
#include "NewsService.hpp"
#include "PriceService.hpp"
#include "../utils/FileIO.hpp"
#include "../utils/Date.hpp"

namespace StockMarketSimulator {

struct SaveMetadata {
    std::string filename;
    std::string displayName;
    Date gameDate;
    double playerNetWorth;
    std::string saveDate;
    bool isAutosave;
    std::string errorMessage;

    SaveMetadata(const std::string& filename = "",
                const std::string& displayName = "",
                const Date& gameDate = Date(),
                double playerNetWorth = 0.0,
                const std::string& saveDate = "",
                bool isAutosave = false);

    nlohmann::json toJson() const;
    static SaveMetadata fromJson(const nlohmann::json& json);
};

class SaveService {
private:
    std::weak_ptr<Market> market;
    std::weak_ptr<Player> player;
    std::weak_ptr<NewsService> newsService;
    std::weak_ptr<PriceService> priceService;

    std::string savesDirectory;
    bool autosaveEnabled;
    int autosaveInterval;
    Date lastAutosaveDate;

    nlohmann::json createSaveData() const;
    bool validateSaveData(const nlohmann::json& saveData) const;
    std::string generateSaveFilename(const std::string& displayName, bool isAutosave) const;
    std::string getCurrentDateTimeString() const;

public:
    SaveService();
    SaveService(std::weak_ptr<Market> market,
                std::weak_ptr<Player> player,
                std::weak_ptr<NewsService> newsService,
                std::weak_ptr<PriceService> priceService);

    void initialize(const std::string& savesDirectory = "data/saves");

    bool saveGame(const std::string& displayName, bool isAutosave = false);
    bool loadGame(const std::string& filename);
    bool deleteSave(const std::string& filename);
    
    std::vector<SaveMetadata> listSaves() const;
    SaveMetadata getSaveMetadata(const std::string& filename) const;
    
    void setAutosave(bool enabled, int interval = 5);
    bool isAutosaveEnabled() const;
    int getAutosaveInterval() const;
    bool checkAndCreateAutosave();
    
    std::string getSavesDirectory() const;
    void setSavesDirectory(const std::string& directory);
    
    void setMarket(std::weak_ptr<Market> market);
    void setPlayer(std::weak_ptr<Player> player);
    void setNewsService(std::weak_ptr<NewsService> newsService);
    void setPriceService(std::weak_ptr<PriceService> priceService);
};

}