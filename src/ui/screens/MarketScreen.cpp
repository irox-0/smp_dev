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

    std::vector<int> columnWidths = {14, 6, 7, 15};
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
    int n = 1;
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
        std::string name =  std::to_string(n) + "." + company->getName();
        std::vector<std::string> row = {
            name,
            priceStr.str(),
            changeStr.str(),
            company->getSectorName()
        };

        companiesTable.addRow(row);
        n++;
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


    Console::setCursorPosition(x + 2, height - 1);
    Console::print("Choose action: ");


}
bool MarketScreen::handleInput(int key) {
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
            return true;
    }
}

void MarketScreen::setNewsService(std::weak_ptr<NewsService> newsService) {
    this->newsService = newsService;
}

std::weak_ptr<NewsService> MarketScreen::getNewsService() const {
    return newsService;
}

const std::vector<std::shared_ptr<Company>>& MarketScreen::getCompanies() const {
    return displayedCompanies;
}

void MarketScreen::viewCompanyDetails(int index) {
    if (index < 0 || index >= static_cast<int>(displayedCompanies.size())) {
        return;
    }

    auto companyPtr = displayedCompanies[index];
    if (!companyPtr) {
        return;
    }

    auto companyScreen = std::make_shared<CompanyScreen>();
    companyScreen->setMarket(market);
    companyScreen->setPlayer(player);
    companyScreen->setCompany(companyPtr);

    companyScreen->setNewsService(newsService);
    Console::clear();
    companyScreen->setPosition(x, y);
    companyScreen->setSize(width, height);

    companyScreen->initialize();

    companyScreen->run();

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