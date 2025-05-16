#include "FileIO.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#include <io.h>
#define ACCESS _access
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#define ACCESS access
#define MKDIR(dir) mkdir(dir, 0755)
#endif

namespace StockMarketSimulator {

bool FileIO::isInitialized = false;

void FileIO::initialize() {
    if (!isInitialized) {
        try {
            if (!directoryExists(getDataDirectory())) {
                createDirectory(getDataDirectory());
            }

            if (!directoryExists(getSavesDirectory())) {
                createDirectory(getSavesDirectory());
            }

            isInitialized = true;
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to initialize FileIO: " + std::string(e.what()));
        }
    }
}

bool FileIO::fileExists(const std::string& filePath) {
    return ACCESS(filePath.c_str(), 0) == 0;
}

nlohmann::json FileIO::readJsonFile(const std::string& filePath) {
    if (!isInitialized) {
        initialize();
    }

    if (!fileExists(filePath)) {
        throw std::runtime_error("File does not exist: " + filePath);
    }

    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }

        nlohmann::json jsonData;
        file >> jsonData;
        file.close();
        return jsonData;
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("JSON parse error in file " + filePath + ": " + e.what());
    } catch (const std::exception& e) {
        throw std::runtime_error("Error reading JSON file " + filePath + ": " + e.what());
    }
}

void FileIO::writeJsonFile(const std::string& filePath, const nlohmann::json& data, bool prettyPrint) {
    if (!isInitialized) {
        initialize();
    }

    try {
        size_t lastSlash = filePath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            std::string directory = filePath.substr(0, lastSlash);
            if (!directory.empty() && !directoryExists(directory)) {
                createDirectory(directory);
            }
        }

        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filePath);
        }

        if (prettyPrint) {
            file << data.dump(4);
        } else {
            file << data.dump();
        }

        file.close();
    } catch (const std::exception& e) {
        throw std::runtime_error("Error writing JSON file " + filePath + ": " + e.what());
    }
}

std::string FileIO::readTextFile(const std::string& filePath) {
    if (!isInitialized) {
        initialize();
    }

    if (!fileExists(filePath)) {
        throw std::runtime_error("File does not exist: " + filePath);
    }

    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        return buffer.str();
    } catch (const std::exception& e) {
        throw std::runtime_error("Error reading text file " + filePath + ": " + e.what());
    }
}

void FileIO::writeTextFile(const std::string& filePath, const std::string& content) {
    if (!isInitialized) {
        initialize();
    }

    try {
        size_t lastSlash = filePath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            std::string directory = filePath.substr(0, lastSlash);
            if (!directory.empty() && !directoryExists(directory)) {
                createDirectory(directory);
            }
        }

        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filePath);
        }

        file << content;
        file.close();
    } catch (const std::exception& e) {
        throw std::runtime_error("Error writing text file " + filePath + ": " + e.what());
    }
}

bool FileIO::platformDirectoryExists(const std::string& directory) {
#ifdef _WIN32
    DWORD fileAttributes = GetFileAttributesA(directory.c_str());
    return (fileAttributes != INVALID_FILE_ATTRIBUTES &&
           (fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat info;
    return (stat(directory.c_str(), &info) == 0 && S_ISDIR(info.st_mode));
#endif
}

void FileIO::platformCreateDirectory(const std::string& directory) {
    std::string currentPath;
    std::string remainingPath = directory;

    std::replace(remainingPath.begin(), remainingPath.end(), '\\', '/');

    size_t pos = 0;
    while ((pos = remainingPath.find('/', pos)) != std::string::npos) {
        currentPath = remainingPath.substr(0, pos);
        if (!currentPath.empty() && !platformDirectoryExists(currentPath)) {
            MKDIR(currentPath.c_str());
        }
        pos++;
    }

    if (!remainingPath.empty() && !platformDirectoryExists(remainingPath)) {
        MKDIR(remainingPath.c_str());
    }
}

std::vector<std::string> FileIO::platformListFiles(const std::string& directory, const std::string& extension) {
    std::vector<std::string> files;

#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    std::string searchPath = directory + "/*";

    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string filename = findData.cFileName;
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                if (extension.empty() ||
                    (filename.length() >= extension.length() &&
                     filename.substr(filename.length() - extension.length()) == extension)) {
                    files.push_back(filename);
                }
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(directory.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                std::string filename = entry->d_name;
                if (extension.empty() ||
                    (filename.length() >= extension.length() &&
                     filename.substr(filename.length() - extension.length()) == extension)) {
                    files.push_back(filename);
                }
            }
        }
        closedir(dir);
    }
#endif

    return files;
}

std::vector<std::string> FileIO::listFiles(const std::string& directory, const std::string& extension) {
    if (!isInitialized) {
        initialize();
    }

    std::vector<std::string> files;

    try {
        if (!directoryExists(directory)) {
            return files;
        }

        return platformListFiles(directory, extension);
    } catch (const std::exception& e) {
        throw std::runtime_error("Error listing files in directory " + directory + ": " + e.what());
    }
}

void FileIO::createDirectory(const std::string& directory) {
    platformCreateDirectory(directory);
}

bool FileIO::directoryExists(const std::string& directory) {
    return platformDirectoryExists(directory);
}

std::string FileIO::getDataDirectory() {
    return "data";
}

std::string FileIO::getSavesDirectory() {
    return combineFilePath(getDataDirectory(), "saves");
}

std::string FileIO::combineFilePath(const std::string& directory, const std::string& filename) {
    if (directory.empty()) {
        return filename;
    }

    char lastChar = directory[directory.length() - 1];
    if (lastChar == '/' || lastChar == '\\') {
        return directory + filename;
    } else {
        return directory + "/" + filename;
    }
}

std::string FileIO::getFileExtension(const std::string& filePath) {
    size_t lastDot = filePath.find_last_of('.');
    if (lastDot == std::string::npos) {
        return "";
    }
    return filePath.substr(lastDot);
}

std::string FileIO::getFileName(const std::string& filePath) {
    size_t lastSlash = filePath.find_last_of("/\\");
    if (lastSlash == std::string::npos) {
        return filePath;
    }
    return filePath.substr(lastSlash + 1);
}
void FileIO::appendToLog(const std::string& message) {
    std::ofstream logFile("log.txt", std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    }
}

void FileIO::clearLog() {
    std::ofstream logFile("log.txt", std::ios::trunc);
    if (logFile.is_open()) {
        logFile.close();
    }
}
}