#include "MarketScreen.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace StockMarketSimulator {

MarketScreen::MarketScreen()
    : Screen("Market", ScreenType::Market),
      sortCriteria(MarketSortCriteria::PriceChangePercent),
      sortAscending(false)
{
    setSize(47, 31);
}

void MarketScreen::initialize() {
    Screen::initialize();

    companiesTable.setPosition(x, y + 3);

    std::vector<std::string> headers = {"Company", "Price", "Change", "Sector"};
    companiesTable.setHeaders(headers);

    std::vector<int> columnWidths = {12, 7, 8, 15};
    companiesTable.setColumnWidths(columnWidths);

    companiesTable.setHeaderColors(TextColor::White, TextColor::Blue);
    companiesTable.setBodyColors(bodyFg, bodyBg);

    updateDisplayedCompanies();
}

void MarketScreen::update() {
    Screen::update();
    updateDisplayedCompanies();
}

void MarketScreen::updateDisplayedCompanies() {
    auto marketPtr = market.lock();
    if (!marketPtr) {
        displayedCompanies.clear();
        return;
    }

    displayedCompanies = marketPtr->getCompanies();

    sortCompanies();
    updateTableData();
}

void MarketScreen::updateTableData() {
    companiesTable.clearData();

    for (const auto& company : displayedCompanies) {
        Stock* stock = company->getStock();
        if (!stock) continue;

        double currentPrice = stock->getCurrentPrice();
        double priceChangePercent = stock->getDayChangePercent();

        std::stringstream priceStr;
        priceStr << std::fixed << std::setprecision(2) << currentPrice;

        std::stringstream changeStr;
        if (priceChangePercent >= 0) {
            changeStr << "+" << std::fixed << std::setprecision(2) << priceChangePercent << "%";
        } else {
            changeStr << std::fixed << std::setprecision(2) << priceChangePercent << "%";
        }

        std::vector<std::string> row = {
            company->getName(),
            priceStr.str(),
            changeStr.str(),
            company->getSectorName()
        };

        companiesTable.addRow(row);
    }
}

void MarketScreen::sortCompanies() {
    switch (sortCriteria) {
        case MarketSortCriteria::Name:
            std::sort(displayedCompanies.begin(), displayedCompanies.end(),
                     [this](const auto& a, const auto& b) {
                         bool result = a->getName() < b->getName();
                         return sortAscending ? result : !result;
                     });
            break;

        case MarketSortCriteria::Price:
            std::sort(displayedCompanies.begin(), displayedCompanies.end(),
                     [this](const auto& a, const auto& b) {
                         double priceA = a->getStock() ? a->getStock()->getCurrentPrice() : 0.0;
                         double priceB = b->getStock() ? b->getStock()->getCurrentPrice() : 0.0;
                         bool result = priceA < priceB;
                         return sortAscending ? result : !result;
                     });
            break;

        case MarketSortCriteria::PriceChange:
            std::sort(displayedCompanies.begin(), displayedCompanies.end(),
                     [this](const auto& a, const auto& b) {
                         double changeA = a->getStock() ? a->getStock()->getDayChangeAmount() : 0.0;
                         double changeB = b->getStock() ? b->getStock()->getDayChangeAmount() : 0.0;
                         bool result = changeA < changeB;
                         return sortAscending ? result : !result;
                     });
            break;

        case MarketSortCriteria::PriceChangePercent:
            std::sort(displayedCompanies.begin(), displayedCompanies.end(),
                     [this](const auto& a, const auto& b) {
                         double percentA = a->getStock() ? a->getStock()->getDayChangePercent() : 0.0;
                         double percentB = b->getStock() ? b->getStock()->getDayChangePercent() : 0.0;
                         bool result = percentA < percentB;
                         return sortAscending ? result : !result;
                     });
            break;

        case MarketSortCriteria::Sector:
            std::sort(displayedCompanies.begin(), displayedCompanies.end(),
                     [this](const auto& a, const auto& b) {
                         bool result = a->getSectorName() < b->getSectorName();
                         return sortAscending ? result : !result;
                     });
            break;
    }
}

void MarketScreen::drawContent() const {
    drawSortInfo();

    Console::setCursorPosition(x, y + 3);
    companiesTable.draw();

    drawNavigationOptions();
}

void MarketScreen::drawSortInfo() const {
    Console::setCursorPosition(x - 1, y + 2);
    Console::setColor(bodyFg, bodyBg);

    std::string sortName;
    switch (sortCriteria) {
        case MarketSortCriteria::Name: sortName = "Company Name"; break;
        case MarketSortCriteria::Price: sortName = "Price"; break;
        case MarketSortCriteria::PriceChange: sortName = "Price Change"; break;
        case MarketSortCriteria::PriceChangePercent: sortName = "Price Change %"; break;
        case MarketSortCriteria::Sector: sortName = "Sector"; break;
    }

    std::string direction = sortAscending ? "ascending" : "descending";
    std::string sortInfo = " Sorted by: " + sortName + " (" + direction + ")";

    if (sortInfo.length() > width - 3) {
        sortInfo = sortInfo.substr(0, width - 6) + "...";
    }

    Console::setCursorPosition(x + 1, y + 2);
    Console::setColor(TextColor::White, bodyBg);
    Console::print(sortInfo);

    int spaceCount = width - 3 - sortInfo.length();
    if (spaceCount > 0) {
        Console::print(std::string(spaceCount, ' '));
    }

    Console::setCursorPosition(x + width - 1, y + 2);
    Console::setColor(bodyFg, bodyBg);
    Console::print("|");

}

void MarketScreen::drawNavigationOptions() const {
    int tableBottom = 2 + companiesTable.calculateTableHeight();

    Console::setCursorPosition(x + 2, tableBottom + 1);
    Console::print("Enter company number for details");

    Console::setCursorPosition(x + 2, tableBottom + 2);
    Console::print("or 0 to return to main menu:");

    Console::setCursorPosition(x + 2, tableBottom + 4);
    Console::print("S - Change Sort Criteria");

    Console::setCursorPosition(x + 2,  tableBottom + 6);
    Console::print("D - Toggle Sort Direction");
    Console::setCursorPosition(x, y + 32);


}
bool MarketScreen::handleInput(int key) {
    // Number keys for company selection
    if (key >= '1' && key <= '9') {
        int companyIndex = key - '1';
        if (companyIndex >= 0 && companyIndex < static_cast<int>(displayedCompanies.size())) {
            viewCompanyDetails(companyIndex);
        }
        return true;
    }

    switch (key) {
        case '0':
        case 27:
            close();
            return false;

        case 's':
        case 'S':
            changeSortCriteria();
            return true;

        case 'd':
        case 'D':
            toggleSortDirection();
            return true;

        default:
            return Screen::handleInput(key);
    }
}

void MarketScreen::viewCompanyDetails(int index) {
    if (index < 0 || index >= static_cast<int>(displayedCompanies.size())) {
        return;
    }

    auto company = displayedCompanies[index];
    auto stock = company->getStock();

    int startY = y + 4;
    int contentHeight = companiesTable.calculateTableHeight() - 2;

    int currentY = startY + 1;

    int messageY = y + 4;
    Console::setCursorPosition(x, messageY);
    Console::setColor(TextColor::Yellow, bodyBg);
    Console::print("|  Selected company: " + company->getName() + " (" + company->getTicker() + ")");
    Console::print(std::string(width - company->getName().length() - 25 - company->getTicker().length(), ' ') + "|");

    for (int i = 0; i < height - 7; i++) {
        Console::setCursorPosition(x, messageY + 2 + i);
        Console::print("|" + std::string(width - 2, ' ') + "|");
    }

    currentY += 2;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Sector: ");
    Console::setColor(TextColor::Cyan, bodyBg);
    Console::print(company->getSectorName());
    currentY += 2;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Current Price: ");
    Console::setColor(TextColor::Yellow, bodyBg);

    double currentPrice = stock->getCurrentPrice();
    std::stringstream priceStr;
    priceStr << std::fixed << std::setprecision(2) << currentPrice << "$";
    Console::print(priceStr.str());
    currentY += 2;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Daily Change: ");

    double changeAmount = stock->getDayChangeAmount();
    double changePercent = stock->getDayChangePercent();

    std::stringstream changeStr;
    changeStr << std::fixed << std::setprecision(2);

    if (changeAmount >= 0) {
        Console::setColor(TextColor::Green, bodyBg);
        changeStr << "+" << changeAmount << "$ (+" << changePercent << "%)";
    } else {
        Console::setColor(TextColor::Red, bodyBg);
        changeStr << changeAmount << "$ (" << changePercent << "%)";
    }

    Console::print(changeStr.str());
    currentY += 2;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("52-week Range: ");
    Console::setColor(TextColor::White, bodyBg);

    std::stringstream rangeStr;
    rangeStr << std::fixed << std::setprecision(2);
    rangeStr << stock->getLowestPrice() << "$ - " << stock->getHighestPrice() << "$";
    Console::print(rangeStr.str());
    currentY += 2;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Volatility: ");

    double volatility = company->getVolatility();
    std::string volatilityDesc;

    if (volatility < 0.3) {
        Console::setColor(TextColor::Green, bodyBg);
        volatilityDesc = "Low";
    } else if (volatility < 0.6) {
        Console::setColor(TextColor::Yellow, bodyBg);
        volatilityDesc = "Medium";
    } else {
        Console::setColor(TextColor::Red, bodyBg);
        volatilityDesc = "High";
    }

    Console::print(volatilityDesc);
    currentY += 2;

    const auto& dividendPolicy = company->getDividendPolicy();
    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Dividends: ");

    if (dividendPolicy.annualDividendRate > 0) {
        std::stringstream divStr;
        divStr << std::fixed << std::setprecision(2);
        divStr << dividendPolicy.annualDividendRate << "$ (";

        if (dividendPolicy.paymentFrequency == 4) {
            divStr << "quarterly";
        } else if (dividendPolicy.paymentFrequency == 2) {
            divStr << "semi-annual";
        } else if (dividendPolicy.paymentFrequency == 1) {
            divStr << "annual";
        } else {
            divStr << dividendPolicy.paymentFrequency << " times per year";
        }

        divStr << ")";
        Console::setColor(TextColor::Cyan, bodyBg);
        Console::print(divStr.str());
    } else {
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("None");
    }
    currentY += 2;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Description:");
    currentY += 1;

    std::string description = company->getDescription();
    int maxLineWidth = width - 6;

    for (size_t pos = 0; pos < description.length(); pos += maxLineWidth) {
        std::string line = description.substr(pos, maxLineWidth);

        Console::setCursorPosition(x + 4, currentY);
        Console::setColor(TextColor::White, bodyBg);
        Console::print(line);
        currentY += 1;

        if (currentY >= startY + contentHeight - 2) {
            break;
        }
    }

    Console::setCursorPosition(x + 2, height - 3);
    Console::setColor(TextColor::Yellow, bodyBg);
    Console::print("Press any key to return to market view");

    Console::readChar();

    draw();
}
MarketSortCriteria MarketScreen::getSortCriteria() const {
    return sortCriteria;
}

void MarketScreen::setSortCriteria(MarketSortCriteria criteria) {
    sortCriteria = criteria;
    sortCompanies();
    updateTableData();
}

bool MarketScreen::isSortAscending() const {
    return sortAscending;
}

void MarketScreen::setSortAscending(bool ascending) {
    sortAscending = ascending;
    sortCompanies();
    updateTableData();
}

void MarketScreen::changeSortCriteria() {
    // Define sort options
    std::vector<std::string> options = {
        "Company Name",
        "Price",
        "Price Change",
        "Price Change %",
        "Sector"
    };

    int numDataRows = static_cast<int>(displayedCompanies.size());
    int tableHeight = companiesTable.getHasHeader() ? 3 + (2 * numDataRows) : 1 + (2 * numDataRows);
    int tableBottom = (y + 4) + tableHeight;

    int menuY = tableBottom - 2;

    Console::setCursorPosition(x, menuY + 1);
    Console::setColor(TextColor::Cyan, bodyBg);
    Console::print("| SELECT SORT CRITERIA:");
    Console::setCursorPosition(x + width - 1, menuY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("|");

    menuY++;

    int selected = static_cast<int>(sortCriteria);
    bool running = true;

    while (running) {
        for (size_t i = 0; i < options.size(); i++) {
            Console::setCursorPosition(x, menuY + static_cast<int>(i));
            Console::setColor(bodyFg, bodyBg);

            if (i == static_cast<size_t>(selected)) {
                Console::print("| > " + options[i]);
            } else {
                Console::print("|   " + options[i]);
            }

            int fillLength = width - 5 - options[i].length();
            if (fillLength > 0) {
                Console::print(std::string(fillLength, ' '));
            }

            Console::setCursorPosition(x + width - 1, menuY + static_cast<int>(i));
            Console::print("|");
        }

        char key = Console::readChar();

        switch (key) {
            case static_cast<char>(Key::ArrowUp):
                selected = (selected > 0) ? selected - 1 : static_cast<int>(options.size()) - 1;
                break;

            case static_cast<char>(Key::ArrowDown):
                selected = (selected < static_cast<int>(options.size()) - 1) ? selected + 1 : 0;
                break;

            case static_cast<char>(Key::Enter):
                if (selected >= 0 && selected < static_cast<int>(options.size())) {
                    setSortCriteria(static_cast<MarketSortCriteria>(selected));
                }
                running = false;
                break;

            case static_cast<char>(Key::Escape):
                running = false;
                break;

            default:
                if (key >= '1' && key <= '5' && (key - '1') < static_cast<int>(options.size())) {
                    selected = key - '1';
                    setSortCriteria(static_cast<MarketSortCriteria>(selected));
                    running = false;
                }
                break;
        }
    }

    draw();
}
void MarketScreen::toggleSortDirection() {
    setSortAscending(!sortAscending);
    draw();
}

}