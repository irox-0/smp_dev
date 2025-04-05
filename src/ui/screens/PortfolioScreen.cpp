#include "PortfolioScreen.hpp"
#include "../../core/Player.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace StockMarketSimulator {

PortfolioScreen::PortfolioScreen()
    : Screen("MY PORTFOLIO", ScreenType::Portfolio),
      currentPage(0),
      itemsPerPage(3),
      totalPages(1)
{
    setSize(50, 38);
}

void PortfolioScreen::initialize() {
    Screen::initialize();

    portfolioTable.setPosition(x, y + 8);
    std::vector<std::string> headers = {"Stock", "Qty", "Avg.Price", "Cur.Price", "P/L"};
    portfolioTable.setHeaders(headers);
    std::vector<int> columnWidths = {11, 6, 9, 10, 8};
    portfolioTable.setColumnWidths(columnWidths);
    portfolioTable.setHeaderColors(TextColor::White, TextColor::Blue);
    portfolioTable.setBodyColors(bodyFg, bodyBg);

    valueChart.setPosition(x + 2, y + 25);
    valueChart.setSize(width - 4, 6);
    valueChart.setTitle("");
    valueChart.setColor(TextColor::Green);

    std::vector<std::string> labels;
    auto marketPtr = market.lock();
    if (marketPtr) {
        Date currentDate = marketPtr->getCurrentDate();
        for (int i = 30; i >= 0; i -= 7) {
            Date labelDate = currentDate;
            labelDate.advanceDays(-i);
            labels.push_back(labelDate.toShortString());
        }
        valueChart.setXLabels(labels);
    } else {
        valueChart.setXLabels({"", "10d", "20d", "30d"});
    }


    update();
}

void PortfolioScreen::update() {
    Screen::update();

    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    Portfolio* portfolio = playerPtr->getPortfolio();
    if (!portfolio) {
        return;
    }

    int totalPositions = static_cast<int>(portfolio->getPositions().size());
    totalPages = std::max(1, (totalPositions + itemsPerPage - 1) / itemsPerPage);

    if (currentPage >= totalPages) {
        currentPage = totalPages - 1;
    }

    updateTableData();

    const std::vector<double>& historyValues = portfolio->getValueHistory();

    std::vector<double> displayData;
    int historySize = static_cast<int>(historyValues.size());
    int numPoints = std::min(30, historySize);

    if (numPoints > 0) {
        int startIndex = (historySize > numPoints) ? historySize - numPoints : 0;
        for (int i = startIndex; i < historySize; ++i) {
            displayData.push_back(historyValues[i]);
        }
    }

    valueChart.setData(displayData);
}

void PortfolioScreen::drawContent() const {
    drawPortfolioInfo();
    drawPortfolioTable();
    drawSectorDistribution();
    drawValueChart();
    drawNavigationOptions();
}

void PortfolioScreen::drawPortfolioInfo() const {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    Portfolio* portfolio = playerPtr->getPortfolio();
    if (!portfolio) {
        return;
    }

    int currentY = y + 2;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Total Value: ");
    Console::setColor(TextColor::Green, bodyBg);

    std::stringstream valueStr;
    valueStr << std::fixed << std::setprecision(2) << portfolio->getTotalValue() << "$";
    Console::print(valueStr.str());
    currentY += 1;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Daily Change: ");

    double dayChange = portfolio->getDayChangeAmount();
    double dayChangePercent = portfolio->getDayChangePercent();

    std::stringstream changeStr;
    if (dayChange >= 0) {
        Console::setColor(TextColor::Green, bodyBg);
        changeStr << "+" << std::fixed << std::setprecision(2) << dayChange << "$ (+"
                << std::fixed << std::setprecision(2) << dayChangePercent << "%)";
    } else {
        Console::setColor(TextColor::Red, bodyBg);
        changeStr << std::fixed << std::setprecision(2) << dayChange << "$ ("
                << std::fixed << std::setprecision(2) << dayChangePercent << "%)";
    }
    Console::print(changeStr.str());
    currentY += 1;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Return (total): ");

    double totalReturn = portfolio->getTotalReturn();
    double totalReturnPercent = portfolio->getTotalReturnPercent();

    std::stringstream returnStr;
    if (totalReturn >= 0) {
        Console::setColor(TextColor::Green, bodyBg);
        returnStr << "+" << std::fixed << std::setprecision(2) << totalReturn << "$ (+"
                << std::fixed << std::setprecision(1) << totalReturnPercent << "%)";
    } else {
        Console::setColor(TextColor::Red, bodyBg);
        returnStr << std::fixed << std::setprecision(2) << totalReturn << "$ ("
                << std::fixed << std::setprecision(1) << totalReturnPercent << "%)";
    }
    Console::print(returnStr.str());
    currentY += 1;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Available Funds: ");
    Console::setColor(TextColor::Yellow, bodyBg);

    std::stringstream fundsStr;
    fundsStr << std::fixed << std::setprecision(2) << portfolio->getCashBalance() << "$";
    Console::print(fundsStr.str());
    currentY += 1;

    // Display margin loan information
    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Margin Loan: ");

    double marginLoan = playerPtr->getMarginLoan();

    if (marginLoan > 0) {
        Console::setColor(TextColor::Red, bodyBg);
    } else {
        Console::setColor(TextColor::Green, bodyBg);
    }

    std::stringstream marginLoanStr;
    marginLoanStr << std::fixed << std::setprecision(2) << marginLoan << "$";
    Console::print(marginLoanStr.str());
    currentY += 1;

    // Display leverage information if using margin
    if (marginLoan > 0) {
        Console::setCursorPosition(x + 2, currentY);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Current Leverage: ");

        double portfolioValue = portfolio->getTotalValue();
        double ownedValue = portfolioValue - marginLoan;
        double leverage = (ownedValue > 0) ? portfolioValue / ownedValue : 999.0;

        // Color based on risk level
        if (leverage > 2.5) {
            Console::setColor(TextColor::Red, bodyBg);
        } else if (leverage > 1.75) {
            Console::setColor(TextColor::Yellow, bodyBg);
        } else {
            Console::setColor(TextColor::Green, bodyBg);
        }

        std::stringstream leverageStr;
        leverageStr << std::fixed << std::setprecision(2) << leverage << "x";
        Console::print(leverageStr.str());
    }

    Console::setCursorPosition(x, y + 8);
    Console::setColor(bodyFg, bodyBg);
    Console::drawHorizontalLine(x, y + 8, width);

    Console::setCursorPosition(x + 2, y + 9);
    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);
    Console::print("Portfolio Structure:");
    Console::setStyle(TextStyle::Regular);
}

void PortfolioScreen::drawPortfolioTable() const {
    portfolioTable.draw();

    auto playerPtr = player.lock();
    if (playerPtr && playerPtr->getPortfolio() && !playerPtr->getPortfolio()->getPositions().empty() ) {
        Console::setCursorPosition(x + 2, y + 17);
        Console::setColor(bodyFg, bodyBg);
        std::stringstream pageInfo;
        pageInfo << "Page " << (currentPage + 1) << " of " << totalPages
                 << " (Press [ and ] to navigate pages)";
        Console::print(pageInfo.str());
    }

    Console::setCursorPosition(x, y + 16);
    Console::setColor(bodyFg, bodyBg);
    Console::drawHorizontalLine(x, y + 16, width);
}

void PortfolioScreen::drawSectorDistribution() const {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    Portfolio* portfolio = playerPtr->getPortfolio();
    if (!portfolio) {
        return;
    }
    Console::drawHorizontalLine(0, x + 18, width);
    Console::setCursorPosition(x + 2, y + 19);
    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);
    Console::print("Sector Distribution:");
    Console::setStyle(TextStyle::Regular);

    std::map<Sector, double> allocation = portfolio->getSectorAllocation();
    double totalStocksValue = portfolio->getTotalStocksValue();

    if (totalStocksValue <= 0.0) {
        Console::setCursorPosition(x + 4, y + 19);
        Console::setColor(bodyFg, bodyBg);
        Console::print("No stocks in portfolio");
        return;
    }

    int currentY = y + 20;
    for (const auto& [sector, value] : allocation) {
        if (value > 0) {
            Console::setCursorPosition(x + 4, currentY);
            Console::setColor(bodyFg, bodyBg);

            std::string sectorName = Market::sectorToString(sector);
            double percentage = (value / totalStocksValue) * 100.0;

            std::stringstream sectorStr;
            sectorStr << sectorName << ": " << std::fixed << std::setprecision(1) << percentage << "%";
            Console::print(sectorStr.str());

            currentY += 1;
        }
    }

    Console::setCursorPosition(x, y + 23);
    Console::setColor(bodyFg, bodyBg);
    Console::drawHorizontalLine(x, y + 23, width);
}

void PortfolioScreen::drawValueChart() const {
    Console::setCursorPosition(x + 2, y + 24);
    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);
    Console::print("Portfolio Value Chart (30 days):");
    Console::setStyle(TextStyle::Regular);
    Console::setCursorPosition(x + 2, y + 25);
    valueChart.draw();


    Console::setCursorPosition(x, y + 32);
    Console::setColor(bodyFg, bodyBg);
    Console::drawHorizontalLine(x, y + 32, width);
}

void PortfolioScreen::drawNavigationOptions() const {
    int currentY = y + 33;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("1. View Stock Details");

    Console::setCursorPosition(x + 2, currentY + 1);
    Console::print("2. Sell Stocks");

    Console::setCursorPosition(x + 2, currentY + 2);
    Console::print("3. Return to Main Menu");


    Console::setCursorPosition(x + 2,   height - 1);
    Console::print("Choose action: ");

}

bool PortfolioScreen::handleInput(int key) {
    switch (key) {
        case '1':
            viewStockDetails();
            return true;

        case '2':
            sellStocks();
            return true;

        case '3':
        case 27:
            close();
            return false;

        case '[':
            previousPage();
            return true;

        case ']':
            nextPage();
            return true;

        default:
            return Screen::handleInput(key);
    }
}

void PortfolioScreen::updateTableData() {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    Portfolio* portfolio = playerPtr->getPortfolio();
    if (!portfolio) {
        return;
    }

    portfolioTable.clearData();

    std::vector<std::pair<std::string, PortfolioPosition>> positions;
    for (const auto& [ticker, position] : portfolio->getPositions()) {
        positions.push_back({ticker, position});
    }

    int startIndex = currentPage * itemsPerPage;
    int endIndex = std::min(startIndex + itemsPerPage, static_cast<int>(positions.size()));

    for (int i = startIndex; i < endIndex; i++) {
        const auto& [ticker, position] = positions[i];

        std::stringstream qtyStr;
        qtyStr << position.quantity;

        std::stringstream avgPriceStr;
        avgPriceStr << std::fixed << std::setprecision(2) << position.averagePurchasePrice;

        std::stringstream curPriceStr;
        curPriceStr << std::fixed << std::setprecision(2) << position.company->getStock()->getCurrentPrice();

        std::stringstream plStr;
        if (position.unrealizedProfitLossPercent >= 0) {
            plStr << "+" << std::fixed << std::setprecision(1) << position.unrealizedProfitLossPercent << "%";
        } else {
            plStr << std::fixed << std::setprecision(1) << position.unrealizedProfitLossPercent << "%";
        }

        std::vector<std::string> row = {
            position.company->getName(),
            qtyStr.str(),
            avgPriceStr.str(),
            curPriceStr.str(),
            plStr.str()
        };

        portfolioTable.addRow(row);
    }
}

void PortfolioScreen::nextPage() {
    if (currentPage < totalPages - 1) {
        currentPage++;
        updateTableData();
        draw();
    }
}

void PortfolioScreen::previousPage() {
    if (currentPage > 0) {
        currentPage--;
        updateTableData();
        draw();
    }
}

void PortfolioScreen::viewStockDetails() {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    Portfolio* portfolio = playerPtr->getPortfolio();
    if (!portfolio) {
        return;
    }

    if (portfolio->getPositions().empty()) {
        int messageY = y + 18;

        for (int i = 0; i < 3; i++) {
            Console::setCursorPosition(x + 2, messageY + i);
            Console::setColor(bodyFg, bodyBg);
            Console::print(std::string(width - 4, ' '));
        }

        Console::setCursorPosition(x + 2, messageY + 1);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("No stocks in portfolio to view.");

        Console::setCursorPosition(x + 2, messageY + 2);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Press any key to continue...");

        Console::readChar();
        draw();
        return;
    }

    std::vector<std::shared_ptr<Company>> companies;

    std::vector<std::pair<std::string, PortfolioPosition>> positions;
    for (const auto& [ticker, position] : portfolio->getPositions()) {
        positions.push_back({ticker, position});
        companies.push_back(position.company);
    }

    std::vector<std::string> options;
    for (const auto& option : companies) {
        std::stringstream optionStr;
        options.push_back(option->getName());
    }
    options.push_back("Cancel");
    int selectedIndex = 0;
    bool running = true;
    int menuY = y + 7;


    while (running) {
        for (int i = 0; i < 16; i++) {
            Console::setCursorPosition(x, menuY + i);
            Console::setColor(bodyFg, bodyBg);
            Console::print("|" + std::string(width - 2, ' ') + "|");
        }

        Console::setCursorPosition(x + 2, menuY);
        Console::setColor(TextColor::White, bodyBg);
        Console::setStyle(TextStyle::Bold);
        Console::print("SELECT STOCK TO VIEW:");
        Console::setStyle(TextStyle::Regular);

        for (size_t i = 0; i < options.size(); i++) {
            Console::setCursorPosition(x + 2, menuY + 2 + static_cast<int>(i));

            if (i == static_cast<size_t>(selectedIndex)) {
                Console::setColor(TextColor::Black, TextColor::White);
                Console::print("> " + options[i]);
            } else {
                Console::setColor(bodyFg, bodyBg);
                Console::print("  " + options[i]);
            }

            int paddingLength = width - 4 - options[i].length() - 2;
            if (paddingLength > 0) {
                Console::print(std::string(paddingLength, ' '));
            }
        }

        Console::setCursorPosition(x + 2, menuY + 2 + static_cast<int>(options.size()) + 1);
        Console::setColor(bodyFg, bodyBg);

        Console::resetAttributes();

        char key = Console::readChar();

        switch (key) {
            case static_cast<char>(Key::ArrowUp):
                selectedIndex = (selectedIndex > 0) ? selectedIndex - 1 : static_cast<int>(options.size()) - 1;
                break;

            case static_cast<char>(Key::ArrowDown):
                selectedIndex = (selectedIndex < static_cast<int>(options.size()) - 1) ? selectedIndex + 1 : 0;
                break;

            case static_cast<char>(Key::Enter):
                running = false;
                break;

            case static_cast<char>(Key::Escape):
                selectedIndex = options.size() - 1;
                running = false;
                break;

            default:
                break;
        }
    }
    if (selectedIndex == static_cast<int>(options.size()) - 1) {
        draw();
        return;
    }
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(companies.size())) {
        auto companyScreen = std::make_shared<CompanyScreen>();
        companyScreen->setMarket(market);
        companyScreen->setPlayer(player);
        companyScreen->setCompany(companies[selectedIndex]);

        companyScreen->setPosition(x, y);
        Console::clear();
        companyScreen->initialize();
        companyScreen->run();
    }
    update();

    draw();
}

void PortfolioScreen::sellStocks() {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    Portfolio* portfolio = playerPtr->getPortfolio();
    if (!portfolio) {
        return;
    }

    if (portfolio->getPositions().empty()) {
        int messageY = y + 7;

        for (int i = 0; i < 16; i++) {
            Console::setCursorPosition(x, messageY + i);
            Console::setColor(bodyFg, bodyBg);
            Console::print("|" + std::string(width - 2, ' ') + "|");
        }

        Console::setCursorPosition(x + 2, messageY + 1);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("No stocks in portfolio to sell.");

        Console::setCursorPosition(x + 2, messageY + 2);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Press any key to continue...");

        Console::readChar();
        draw();
        return;
    }

    std::vector<std::shared_ptr<Company>> companies;
    for (const auto& [ticker, position] : portfolio->getPositions()) {
        companies.push_back(position.company);
    }

    int selectedIndex = 0;
    bool running = true;
    int menuY = y + 7;

    std::vector<std::string> options;
    for (const auto& option : companies) {
        std::stringstream optionStr;
        options.push_back(option->getName());
    }
    options.push_back("Cancel");
    while (running) {
        for (int i = 0; i < 16; i++) {
            Console::setCursorPosition(x, menuY + i);
            Console::setColor(bodyFg, bodyBg);
            Console::print("|" + std::string(width - 2, ' ') + "|");
        }

        Console::setCursorPosition(x + 2, menuY);
        Console::setColor(TextColor::White, bodyBg);
        Console::setStyle(TextStyle::Bold);
        Console::print("SELECT STOCK TO SELL:");
        Console::setStyle(TextStyle::Regular);




        for (size_t i = 0; i < options.size(); i++) {
            Console::setCursorPosition(x + 2, menuY + 2 + static_cast<int>(i));

            if (i == static_cast<size_t>(selectedIndex)) {
                Console::setColor(TextColor::Black, TextColor::White);
                Console::print("> " + options[i]);
            } else {
                Console::setColor(bodyFg, bodyBg);
                Console::print("  " + options[i]);
            }

            int paddingLength = width - 4 - options[i].length() - 2;
            if (paddingLength > 0) {
                Console::print(std::string(paddingLength, ' '));
            }
        }

        Console::setCursorPosition(x + 2, menuY + 2 + static_cast<int>(companies.size()) + 1);
        Console::setColor(bodyFg, bodyBg);

        Console::resetAttributes();

        char key = Console::readChar();

        switch (key) {
            case static_cast<char>(Key::ArrowUp):
                selectedIndex = (selectedIndex > 0) ? selectedIndex - 1 : static_cast<int>(options.size()) - 1;
                break;

            case static_cast<char>(Key::ArrowDown):
                selectedIndex = (selectedIndex < static_cast<int>(options.size()) - 1) ? selectedIndex + 1 : 0;
                break;

            case static_cast<char>(Key::Enter):
                running = false;
                break;

            case static_cast<char>(Key::Escape):
                selectedIndex = static_cast<int>(options.size()) - 1;
                running = false;
                break;

            default:
                break;
        }
    }

    if (selectedIndex == static_cast<int>(options.size()) - 1) {
        draw();
        return;
    }

    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(companies.size())) {
        auto company = companies[selectedIndex];
        int ownedShares = portfolio->getPositionQuantity(company->getTicker());

        int sellY = menuY;

        for (int i = 0; i < 8; i++) {
            Console::setCursorPosition(x + 1, sellY + i);
            Console::setColor(bodyFg, bodyBg);
            Console::print(std::string(width - 2, ' '));
        }

        Console::setCursorPosition(x + 2, sellY);
        Console::setColor(TextColor::White, bodyBg);
        Console::setStyle(TextStyle::Bold);
        Console::print("SELL STOCKS: " + company->getName());
        Console::setStyle(TextStyle::Regular);

        Console::setCursorPosition(x + 2, sellY + 2);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Current Price: ");
        Console::setColor(TextColor::Yellow, bodyBg);

        double price = company->getStock()->getCurrentPrice();
        std::stringstream priceStr;
        priceStr << std::fixed << std::setprecision(2) << price << "$";
        Console::print(priceStr.str());

        Console::setCursorPosition(x + 2, sellY + 3);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Owned Shares: ");
        Console::setColor(TextColor::Green, bodyBg);
        Console::print(std::to_string(ownedShares));

        Console::setCursorPosition(x + 2, sellY + 5);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Enter quantity to sell (0 to cancel): ");
        Console::setColor(TextColor::Yellow, bodyBg);

        std::string input = Console::readLine();
        int quantity = 0;

        try {
            quantity = std::stoi(input);
        } catch (...) {
            quantity = 0;
        }

        Console::setCursorPosition(x + 2, sellY + 6);

        if (quantity == 0) {
            Console::setColor(TextColor::Yellow, bodyBg);
            Console::print("Transaction canceled.");
        } else if (quantity < 0 || quantity > ownedShares) {
            Console::setColor(TextColor::Red, bodyBg);
            Console::print("Invalid quantity! Transaction canceled.");
        } else {
            bool success = playerPtr->sellStock(company, quantity);

            if (success) {
                double totalProceeds = price * quantity * 0.99;

                Console::setColor(TextColor::Green, bodyBg);
                Console::print("Successfully sold " + std::to_string(quantity) + " stocks for ");

                std::stringstream proceedsStr;
                proceedsStr << std::fixed << std::setprecision(2) << totalProceeds << "$";
                Console::print(proceedsStr.str());
            } else {
                Console::setColor(TextColor::Red, bodyBg);
                Console::print("Transaction failed!");
            }
        }

        Console::setCursorPosition(x + 2, sellY + 8);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Press any key to continue...");

        Console::readChar();
        update();
    }

    draw();
}

}