// main.cpp
#include <iostream>
#include <vector>
#include <string>
#include "ui/widgets/Table.hpp"
#include "utils/Console.hpp"

using namespace StockMarketSimulator;

int main() {
    Console::initialize();
    Console::clear();

    std::vector<std::string> headers = {"ID", "Company", "Price", "Change", "Sector"};
    std::vector<int> columnWidths = {5, 20, 10, 10, 15};

    Table marketTable(2, 2, headers, columnWidths);

    marketTable.addRow({"1", "TechCorp", "85.67", "+5.67%", "Technology"});
    marketTable.addRow({"2", "EnergyPlus", "45.30", "+3.21%", "Energy"});
    marketTable.addRow({"3", "BankCo", "32.10", "+1.20%", "Finance"});
    marketTable.addRow({"4", "IndustryCo", "67.80", "+0.70%", "Manufacturing"});
    marketTable.addRow({"5", "RetailGiant", "23.45", "-0.50%", "Consumer"});
    marketTable.addRow({"6", "OilMax", "76.32", "-1.20%", "Energy"});
    marketTable.addRow({"7", "FoodCorp", "34.56", "-1.50%", "Consumer"});
    marketTable.addRow({"8", "ConsumerFirst", "56.78", "-2.87%", "Consumer"});

    Console::println("Рынок акций", TextColor::Green, TextColor::Default, TextStyle::Bold);
    Console::println("Отсортировано по: ID (возрастание)\n");
    marketTable.draw();

    Console::setCursorPosition(2, marketTable.calculateTableHeight() + 5);
    Console::readChar();

    marketTable.setSortFunction([](const std::vector<std::string>& a,
                                 const std::vector<std::string>& b,
                                 int columnIndex) {
        auto extractChangeValue = [](const std::string& s) {
            double value = 0.0;
            try {
                size_t pos = 0;
                while (pos < s.length() && !isdigit(s[pos]) && s[pos] != '-' && s[pos] != '+') {
                    pos++;
                }

                std::string numStr = s.substr(pos);
                if (numStr.back() == '%') {
                    numStr.pop_back();
                }

                value = std::stod(numStr);
            } catch (...) {
            }
            return value;
        };

        double valueA = extractChangeValue(a[columnIndex]);
        double valueB = extractChangeValue(b[columnIndex]);

        return valueA < valueB;
    });

    marketTable.sortByColumn(3, false);

    Console::clear();
    Console::println("Рынок акций", TextColor::Green, TextColor::Default, TextStyle::Bold);
    Console::println("Отсортировано по: Изменению цены (убывание)\n");
    marketTable.draw();

    Console::setCursorPosition(2, marketTable.calculateTableHeight() + 5);
    Console::readChar();

    Console::clear();
    Console::println("Мой портфель", TextColor::Green, TextColor::Default, TextStyle::Bold);
    Console::println("Общая стоимость: 8,450.75$   Изменение за день: +120.25$ (+1.44%)\n");

    std::vector<std::string> headers1 = {"Stock", "N", "Avg.price", "Cur.price", "P/L"};
    Table portfolioTable(2, 2, headers1, {10, 10, 10, 10, 10});

    portfolioTable.addRow({"TechCorp", "50", "75.30$", "85.67$", "+10.4%"});
    portfolioTable.addRow({"EnergyPls", "70", "42.20$", "45.30$", "+7.3%"});
    portfolioTable.addRow({"BankCo", "30", "33.10$", "32.10$", "-3.0%"});
    portfolioTable.addRow({"OilMax", "20", "80.50$", "76.32$", "-5.2%"});

    portfolioTable.draw();


     Table sectorTable(2, portfolioTable.calculateTableHeight() + 7,
                      {"Sector", "Share"}, {15, 10});

     sectorTable.addRow({"Technology", "50.7%"});
     sectorTable.addRow({"Energy", "33.1%"});
     sectorTable.addRow({"Finance", "16.2%"});

     sectorTable.draw();


    Console::setCursorPosition(2, sectorTable.calculateTableHeight() + sectorTable.getY() + 2);
    Console::readChar();

    Console::clear();
    return 0;
}