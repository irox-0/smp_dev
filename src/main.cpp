#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "core/Game.hpp"
#include "services/SaveService.hpp"
#include "ui/screens/MainScreen.hpp"
#include "utils/Console.hpp"

using namespace StockMarketSimulator;

void displayWelcomeScreen() {
    Console::clear();

    Console::setCursorPosition(10, 5);
    Console::setColor(TextColor::White, TextColor::Blue);
    Console::print(" STOCK MARKET SIMULATOR ");
    Console::resetAttributes();

    Console::setCursorPosition(15, 8);
    Console::setColor(TextColor::Yellow, TextColor::Default);
    Console::print("Welcome to Stock Player!");
    Console::resetAttributes();

    Console::setCursorPosition(12, 10);
    Console::print("1. Start New Game");

    Console::setCursorPosition(12, 11);
    Console::print("2. Load Saved Game");

    Console::setCursorPosition(12, 12);
    Console::print("0. Exit");

    Console::setCursorPosition(12, 14);
    Console::print("Enter your choice: ");
}

std::string getSaveFilename() {
    Console::clear();

    Console::setCursorPosition(10, 5);
    Console::setColor(TextColor::White, TextColor::Blue);
    Console::print(" LOAD SAVED GAME ");
    Console::resetAttributes();

    std::string savesDirectory = "data/saves";

    std::vector<std::string> saveFiles = FileIO::listFiles(savesDirectory, ".json");

    if (saveFiles.empty()) {
        Console::setCursorPosition(12, 8);
        Console::print("No save files found!");

        Console::setCursorPosition(12, 10);
        Console::print("Press any key to return...");
        Console::readChar();
        return "";
    }

    std::vector<std::string> filteredFiles;
    for (const auto& file : saveFiles) {
        if (file.find(".meta") == std::string::npos) {
            filteredFiles.push_back(file);
        }
    }

    if (filteredFiles.empty()) {
        Console::setCursorPosition(12, 8);
        Console::print("No valid save files found!");

        Console::setCursorPosition(12, 10);
        Console::print("Press any key to return...");
        Console::readChar();
        return "";
    }

    int currentY = 8;
    Console::setCursorPosition(10, currentY);
    Console::print("Available save files:");
    currentY += 2;

    for (size_t i = 0; i < filteredFiles.size(); i++) {
        Console::setCursorPosition(12, currentY);
        Console::print(std::to_string(i + 1) + ". " + filteredFiles[i]);
        currentY++;
    }

    Console::setCursorPosition(12, currentY + 1);
    Console::print("0. Back");

    Console::setCursorPosition(12, currentY + 3);
    Console::print("Enter your choice: ");

    int choice = -1;
    std::string input = Console::readLine();

    try {
        choice = std::stoi(input);
    } catch (...) {
        choice = -1;
    }

    if (choice == 0) {
        return "";
    }

    if (choice >= 1 && choice <= static_cast<int>(filteredFiles.size())) {
        return filteredFiles[choice - 1];
    }

    Console::setCursorPosition(12, currentY + 5);
    Console::setColor(TextColor::Red, TextColor::Default);
    Console::print("Invalid choice! Press any key to return...");
    Console::resetAttributes();
    Console::readChar();

    return "";
}

std::pair<std::string, double> getPlayerInfo() {
    Console::clear();

    Console::setCursorPosition(10, 5);
    Console::setColor(TextColor::White, TextColor::Blue);
    Console::print(" NEW GAME SETUP ");
    Console::resetAttributes();

    Console::setCursorPosition(12, 8);
    Console::print("Enter your name: ");
    std::string playerName = Console::readLine();

    if (playerName.empty()) {
        playerName = "Trader";
    }

    Console::setCursorPosition(12, 10);
    Console::print("Enter initial investment ($): ");
    std::string balanceStr = Console::readLine();

    double initialBalance = 10000.0;

    try {
        if (!balanceStr.empty()) {
            initialBalance = std::stod(balanceStr);
            if (initialBalance <= 0) {
                initialBalance = 10000.0;
            }
        }
    } catch (...) {
    }

    return {playerName, initialBalance};
}

int main() {
    try {
        Console::initialize();

        std::shared_ptr<Game> game = std::make_shared<Game>();

        while (true) {
            displayWelcomeScreen();

            int choice = Console::readChar() - '0';

            switch (choice) {
                case 1: {
                    auto [playerName, initialBalance] = getPlayerInfo();

                    game->initialize(playerName, initialBalance);
                    game->start();

                    auto mainScreen = std::make_shared<MainScreen>();
                    mainScreen->setGame(game);
                    mainScreen->initialize();
                    Console::clear();
                    mainScreen->run();

                    break;
                }

                case 2: {
                    std::string filename = getSaveFilename();

                    if (!filename.empty()) {
                        game->initialize();

                        if (game->loadGame(filename)) {
                            game->start();

                            auto mainScreen = std::make_shared<MainScreen>();
                            mainScreen->setGame(game);
                            mainScreen->initialize();
                            Console::clear();
                            mainScreen->run();
                        } else {
                            Console::clear();
                            Console::setCursorPosition(12, 10);
                            Console::setColor(TextColor::Red, TextColor::Default);
                            Console::print("Error loading save file: " + game->getLastError());
                            Console::resetAttributes();

                            Console::setCursorPosition(12, 12);
                            Console::print("Press any key to continue...");
                            Console::readChar();
                        }
                    }
                    break;
                }

                case 0:
                    Console::clear();
                    Console::setCursorPosition(12, 10);
                    Console::print("Thank you for playing Stock Market Simulator!");
                    Console::setCursorPosition(12, 12);
                    Console::print("Goodbye!");

                    Console::cleanup();
                    return 0;

                default:
                    Console::setCursorPosition(12, 16);
                    Console::setColor(TextColor::Red, TextColor::Default);
                    Console::print("Invalid choice! Press any key to try again...");
                    Console::resetAttributes();
                    Console::readChar();
                    break;
            }
        }

        return 0;
    }
    catch (const std::exception& e) {
        Console::clear();
        Console::setCursorPosition(0, 0);
        Console::setColor(TextColor::Red, TextColor::Default);
        Console::println("Error: " + std::string(e.what()));

        Console::cleanup();
        return 1;
    }
}