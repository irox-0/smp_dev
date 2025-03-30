#include "SaveService.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cstdio>
#include <algorithm>

namespace StockMarketSimulator {

SaveMetadata::SaveMetadata(const std::string& filename,
                          const std::string& displayName,
                          const Date& gameDate,
                          double playerNetWorth,
                          const std::string& saveDate,
                          bool isAutosave)
    : filename(filename),
      displayName(displayName),
      gameDate(gameDate),
      playerNetWorth(playerNetWorth),
      saveDate(saveDate),
      isAutosave(isAutosave)
{
}

nlohmann::json SaveMetadata::toJson() const {
    nlohmann::json j;
    j["filename"] = filename;
    j["display_name"] = displayName;
    j["game_date"] = gameDate.toJson();
    j["player_net_worth"] = playerNetWorth;
    j["save_date"] = saveDate;
    j["is_autosave"] = isAutosave;
    return j;
}

SaveMetadata SaveMetadata::fromJson(const nlohmann::json& json) {
    SaveMetadata metadata;
    metadata.filename = json["filename"];
    metadata.displayName = json["display_name"];

    if (json.contains("game_date")) {
        metadata.gameDate = Date::fromJson(json["game_date"]);
    } else if (json.contains("game_day")) {
        int gameDay = json["game_day"];
        metadata.gameDate = Date::fromDayNumber(gameDay);
    } else {
        metadata.gameDate = Date();
    }

    metadata.playerNetWorth = json["player_net_worth"];
    metadata.saveDate = json["save_date"];
    metadata.isAutosave = json["is_autosave"];
    return metadata;
}

SaveService::SaveService()
    : savesDirectory("data/saves"),
      autosaveEnabled(true),
      autosaveInterval(5),
      lastAutosaveDate()
{
}

SaveService::SaveService(std::weak_ptr<Market> market,
                        std::weak_ptr<Player> player,
                        std::weak_ptr<NewsService> newsService,
                        std::weak_ptr<PriceService> priceService)
    : market(market),
      player(player),
      newsService(newsService),
      priceService(priceService),
      savesDirectory("data/saves"),
      autosaveEnabled(true),
      autosaveInterval(5),
      lastAutosaveDate()
{
}

void SaveService::initialize(const std::string& savesDirectory) {
    this->savesDirectory = savesDirectory;

    if (!FileIO::directoryExists(savesDirectory)) {
        FileIO::createDirectory(savesDirectory);
    }
}

bool SaveService::saveGame(const std::string& displayName, bool isAutosave) {
    nlohmann::json saveData = createSaveData();
    if (saveData.empty()) {
        return false;
    }

    std::string filename = generateSaveFilename(displayName, isAutosave);
    std::string filePath = FileIO::combineFilePath(savesDirectory, filename);

    try {
        FileIO::writeJsonFile(filePath, saveData, true);

        auto playerPtr = player.lock();
        if (playerPtr) {
            SaveMetadata metadata(filename,
                                displayName,
                                playerPtr->getCurrentDate(),
                                playerPtr->getNetWorth(),
                                getCurrentDateTimeString(),
                                isAutosave);

            std::string metadataPath = FileIO::combineFilePath(savesDirectory, filename + ".meta");
            FileIO::writeJsonFile(metadataPath, metadata.toJson(), true);
        }

        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool SaveService::loadGame(const std::string& filename) {
    std::string filePath = FileIO::combineFilePath(savesDirectory, filename);

    if (!FileIO::fileExists(filePath)) {
        return false;
    }

    try {
        nlohmann::json saveData = FileIO::readJsonFile(filePath);

        if (!validateSaveData(saveData)) {
            return false;
        }

        auto marketPtr = market.lock();
        auto playerPtr = player.lock();
        auto newsServicePtr = newsService.lock();
        auto priceServicePtr = priceService.lock();

        if (!marketPtr || !playerPtr) {
            return false;
        }

        *marketPtr = Market::fromJson(saveData["market"]);
        *playerPtr = Player::fromJson(saveData["player"], marketPtr);

        if (newsServicePtr && saveData.contains("news_service")) {
            *newsServicePtr = NewsService::fromJson(saveData["news_service"], marketPtr);
        }

        if (priceServicePtr && saveData.contains("price_service")) {
            *priceServicePtr = PriceService::fromJson(saveData["price_service"], marketPtr);
        }

        lastAutosaveDate = playerPtr->getCurrentDate();

        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool SaveService::deleteSave(const std::string& filename) {
    std::string filePath = FileIO::combineFilePath(savesDirectory, filename);
    std::string metadataPath = FileIO::combineFilePath(savesDirectory, filename + ".meta");

    bool success = true;

    if (FileIO::fileExists(filePath)) {
        std::remove(filePath.c_str());
    } else {
        success = false;
    }

    if (FileIO::fileExists(metadataPath)) {
        std::remove(metadataPath.c_str());
    }

    return success;
}

std::vector<SaveMetadata> SaveService::listSaves() const {
    std::vector<SaveMetadata> saves;

    std::vector<std::string> saveFiles = FileIO::listFiles(savesDirectory, ".json");

    for (const auto& file : saveFiles) {
        if (file.find(".meta") != std::string::npos) {
            continue;
        }

        std::string metadataPath = FileIO::combineFilePath(savesDirectory, file + ".meta");

        if (FileIO::fileExists(metadataPath)) {
            try {
                nlohmann::json metadataJson = FileIO::readJsonFile(metadataPath);
                saves.push_back(SaveMetadata::fromJson(metadataJson));
            } catch (const std::exception& e) {
                std::string displayName = "Metadata error: " + file;
                SaveMetadata errorMetadata(file, displayName, Date(), 0.0, "Damaged file", false);
                saves.push_back(errorMetadata);
            }
        } else {
            std::string displayName = file;
            saves.push_back(SaveMetadata(file, displayName, Date(), 0.0, "", false));
        }
    }

    return saves;
}

SaveMetadata SaveService::getSaveMetadata(const std::string& filename) const {
    std::string metadataPath = FileIO::combineFilePath(savesDirectory, filename + ".meta");

    if (FileIO::fileExists(metadataPath)) {
        try {
            nlohmann::json metadataJson = FileIO::readJsonFile(metadataPath);
            return SaveMetadata::fromJson(metadataJson);
        } catch (const std::exception& e) {
            SaveMetadata errorMetadata(filename, filename, Date(), 0.0, "", false);
            errorMetadata.errorMessage = "Failed to read metadata: " + std::string(e.what());
            return errorMetadata;
        }
    }

    return SaveMetadata(filename, filename, Date(), 0.0, "", false);
}

void SaveService::setAutosave(bool enabled, int interval) {
    autosaveEnabled = enabled;

    if (interval > 0) {
        autosaveInterval = interval;
    }
}

bool SaveService::isAutosaveEnabled() const {
    return autosaveEnabled;
}

int SaveService::getAutosaveInterval() const {
    return autosaveInterval;
}

bool SaveService::checkAndCreateAutosave() {
    if (!autosaveEnabled) {
        return false;
    }

    auto playerPtr = player.lock();
    if (!playerPtr) {
        return false;
    }

    Date currentDate = playerPtr->getCurrentDate();

    int daysDifference = lastAutosaveDate.daysBetween(currentDate);

    if (daysDifference >= autosaveInterval) {
        std::string displayName = "Autosave - " + currentDate.toString();
        bool result = saveGame(displayName, true);  // Pass true for isAutosave flag

        if (result) {
            lastAutosaveDate = currentDate;
        }

        return result;
    }

    return false;
}

std::string SaveService::getSavesDirectory() const {
    return savesDirectory;
}

void SaveService::setSavesDirectory(const std::string& directory) {
    savesDirectory = directory;

    if (!FileIO::directoryExists(directory)) {
        FileIO::createDirectory(directory);
    }
}

void SaveService::setMarket(std::weak_ptr<Market> market) {
    this->market = market;
}

void SaveService::setPlayer(std::weak_ptr<Player> player) {
    this->player = player;
}

void SaveService::setNewsService(std::weak_ptr<NewsService> newsService) {
    this->newsService = newsService;
}

void SaveService::setPriceService(std::weak_ptr<PriceService> priceService) {
    this->priceService = priceService;
}

nlohmann::json SaveService::createSaveData() const {
    nlohmann::json saveData;

    auto marketPtr = market.lock();
    auto playerPtr = player.lock();
    auto newsServicePtr = newsService.lock();
    auto priceServicePtr = priceService.lock();

    if (!marketPtr || !playerPtr) {
        return nlohmann::json();
    }

    saveData["market"] = marketPtr->toJson();
    saveData["player"] = playerPtr->toJson();

    if (newsServicePtr) {
        saveData["news_service"] = newsServicePtr->toJson();
    }

    if (priceServicePtr) {
        saveData["price_service"] = priceServicePtr->toJson();
    }

    saveData["save_version"] = 1;
    saveData["save_date"] = getCurrentDateTimeString();

    return saveData;
}

bool SaveService::validateSaveData(const nlohmann::json& saveData) const {
    if (!saveData.contains("market") || !saveData.contains("player")) {
        return false;
    }

    if (saveData.contains("save_version")) {
        int version = saveData["save_version"];
        if (version > 1) {
            return false;
        }
    }

    return true;
}

std::string SaveService::generateSaveFilename(const std::string& displayName, bool isAutosave) const {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return "save_error.json";
    }

    std::string prefix = isAutosave ? "autosave_" : "save_";
    std::string timestamp = std::to_string(std::time(nullptr));

    Date currentDate = playerPtr->getCurrentDate();
    std::string dateStr = currentDate.toString();

    std::string sanitizedName = displayName;
    std::replace(sanitizedName.begin(), sanitizedName.end(), ' ', '_');
    sanitizedName.erase(std::remove_if(sanitizedName.begin(), sanitizedName.end(),
                                      [](char c) { return !isalnum(c) && c != '_' && c != '-'; }),
                      sanitizedName.end());

    return prefix + dateStr + "_" + sanitizedName + "_" + timestamp + ".json";
}

std::string SaveService::getCurrentDateTimeString() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

}