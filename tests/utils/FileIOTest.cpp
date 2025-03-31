#include <gtest/gtest.h>
#include "../../src/utils/FileIO.hpp"

using namespace StockMarketSimulator;

class FileIOTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir = "test_data";
        FileIO::createDirectory(testDir);
    }

    void TearDown() override {
    }

    std::string testDir;
};

TEST_F(FileIOTest, FileExistsTest) {
    std::string testFile = FileIO::combineFilePath(testDir, "test.txt");

    ASSERT_FALSE(FileIO::fileExists(testFile));

    FileIO::writeTextFile(testFile, "Test content");

    ASSERT_TRUE(FileIO::fileExists(testFile));
}

TEST_F(FileIOTest, ReadWriteTextFileTest) {
    std::string testFile = FileIO::combineFilePath(testDir, "test.txt");
    std::string content = "Test content with special chars: 123!@#$%^&*()";

    FileIO::writeTextFile(testFile, content);

    std::string readContent = FileIO::readTextFile(testFile);

    ASSERT_EQ(content, readContent);

    std::string nonExistentFile = FileIO::combineFilePath(testDir, "nonexistent.txt");
    ASSERT_THROW(FileIO::readTextFile(nonExistentFile), std::runtime_error);
}

TEST_F(FileIOTest, ReadWriteJsonFileTest) {
    std::string testFile = FileIO::combineFilePath(testDir, "test.json");

    nlohmann::json testData = {
        {"name", "Test Company"},
        {"price", 123.45},
        {"active", true},
        {"stats", {
            {"revenue", 1000000},
            {"profit", 500000}
        }},
        {"products", {"Product A", "Product B", "Product C"}}
    };

    FileIO::writeJsonFile(testFile, testData);

    nlohmann::json readData = FileIO::readJsonFile(testFile);

    ASSERT_EQ(testData, readData);

    std::string nonExistentFile = FileIO::combineFilePath(testDir, "nonexistent.json");
    ASSERT_THROW(FileIO::readJsonFile(nonExistentFile), std::runtime_error);

    std::string invalidJsonFile = FileIO::combineFilePath(testDir, "invalid.json");
    FileIO::writeTextFile(invalidJsonFile, "This is not valid JSON");
    ASSERT_THROW(FileIO::readJsonFile(invalidJsonFile), std::runtime_error);
}

TEST_F(FileIOTest, DirectoryOperationsTest) {
    std::string nestedDir = FileIO::combineFilePath(testDir, "nested");

    ASSERT_NO_THROW(FileIO::createDirectory(nestedDir));

    ASSERT_TRUE(FileIO::directoryExists(nestedDir));
}

TEST_F(FileIOTest, FilePathOperationsTest) {
    ASSERT_EQ(FileIO::combineFilePath("dir1", "file.txt"), "dir1/file.txt");
    ASSERT_EQ(FileIO::combineFilePath("dir1/", "file.txt"), "dir1/file.txt");

    ASSERT_EQ(FileIO::getFileExtension("file.txt"), ".txt");
    ASSERT_EQ(FileIO::getFileExtension("file.json"), ".json");
    ASSERT_EQ(FileIO::getFileExtension("file"), "");

    ASSERT_EQ(FileIO::getFileName("/path/to/file.txt"), "file.txt");
    ASSERT_EQ(FileIO::getFileName("file.txt"), "file.txt");
}

TEST_F(FileIOTest, DataDirectoriesTest) {
    ASSERT_FALSE(FileIO::getDataDirectory().empty());
    ASSERT_FALSE(FileIO::getSavesDirectory().empty());

    ASSERT_EQ(FileIO::getSavesDirectory(), FileIO::combineFilePath(FileIO::getDataDirectory(), "saves"));
}