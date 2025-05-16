#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace StockMarketSimulator {

class FileIO {
private:
    static bool isInitialized;
    static void initialize();

    static bool platformDirectoryExists(const std::string& directory);
    static void platformCreateDirectory(const std::string& directory);
    static std::vector<std::string> platformListFiles(const std::string& directory, const std::string& extension);

public:
    static bool fileExists(const std::string& filePath);

    static nlohmann::json readJsonFile(const std::string& filePath);
    static void writeJsonFile(const std::string& filePath, const nlohmann::json& data, bool prettyPrint = true);

    static std::string readTextFile(const std::string& filePath);
    static void writeTextFile(const std::string& filePath, const std::string& content);

    static std::vector<std::string> listFiles(const std::string& directory, const std::string& extension = "");
    static void createDirectory(const std::string& directory);
    static bool directoryExists(const std::string& directory);

    static std::string getDataDirectory();
    static std::string getSavesDirectory();

    static std::string combineFilePath(const std::string& directory, const std::string& filename);
    static std::string getFileExtension(const std::string& filePath);
    static std::string getFileName(const std::string& filePath);
    static void appendToLog(const std::string& message);
    static void clearLog();
};

}