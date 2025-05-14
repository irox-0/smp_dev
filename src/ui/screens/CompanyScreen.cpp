#include "CompanyScreen.hpp"
#include "../../core/Player.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <set>

namespace StockMarketSimulator {

CompanyScreen::CompanyScreen()
    : Screen("Company", ScreenType::Company),
      company(nullptr)
{
    setSize(46, 31);
}

void CompanyScreen::setCompany(std::shared_ptr<Company> company) {
    this->company = company;

    if (company) {
        setTitle(company->getName() + " (" + company->getTicker() + ")");
    } else {
        setTitle("Company");
    }

    update();
}

std::shared_ptr<Company> CompanyScreen::getCompany() const {
    return company;
}

void CompanyScreen::setNewsService(std::weak_ptr<NewsService> newsService) {
    this->newsService = newsService;
}

void CompanyScreen::initialize() {
    Screen::initialize();

    priceChart.setPosition(x + 2, y + 8);
    priceChart.setSize(width - 4, 10);
    priceChart.setTitle("Price Chart");
    priceChart.setColor(TextColor::Green);

    update();
}

void CompanyScreen::update() {
    Screen::update();

    if (!company) {
        return;
    }

    Stock* stock = company->getStock();
    if (!stock) {
        return;
    }

    const std::vector<double>& history = stock->getPriceHistory();
    size_t historySize = history.size();

    std::vector<double> displayData;
    size_t numPoints = std::min(historySize, static_cast<size_t>(90));

    if (numPoints > 0) {
        size_t startIndex = (historySize > numPoints) ? historySize - numPoints : 0;
        for (size_t i = startIndex; i < historySize; ++i) {
            displayData.push_back(history[i]);
        }
    }

    priceChart.setData(displayData);

    auto marketPtr = market.lock();
    if (marketPtr && numPoints > 0) {
        Date currentDate = marketPtr->getCurrentDate();
        Date startDate(1, 3, 2023);
        std::vector<std::string> labels;

        int totalDays = startDate.daysBetween(currentDate) + 1;

        int actualPoints = std::min(static_cast<int>(numPoints), totalDays);

        int maxLabels = 5;
        if (actualPoints <= maxLabels) {
            for (int i = 0; i < actualPoints; i++) {
                Date labelDate = startDate;
                labelDate.advanceDays(i);
                labels.push_back(labelDate.toShortString());
            }
        } else {
            int interval = std::max(1, actualPoints / (maxLabels - 1));
            for (int i = 0; i < actualPoints; i += interval) {
                Date labelDate = startDate;
                labelDate.advanceDays(i);
                labels.push_back(labelDate.toShortString());
                if (labels.size() >= maxLabels - 1) break;
            }
            labels.push_back(currentDate.toShortString());
        }

        priceChart.setXLabels(labels);
    }
}

void CompanyScreen::drawContent() const {
    if (!company) {
        Console::setCursorPosition(x + 2, y + 2);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("No company selected!");
        return;
    }

    drawCompanyInfo();
    drawPriceChart();
    drawLatestNews();
    drawDescription();
    drawActions();
}

void CompanyScreen::drawCompanyInfo() const {
    Stock* stock = company->getStock();
    if (!stock) {
        return;
    }

    int currentY = y + 2;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Sector: ");
    Console::setColor(TextColor::Blue, bodyBg);
    Console::print(company->getSectorName());
    currentY += 1;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Current Price: ");

    double currentPrice = stock->getCurrentPrice();
    double changePercent = stock->getDayChangePercent();

    std::stringstream priceStr;
    priceStr << std::fixed << std::setprecision(2) << currentPrice << "$ ";

    if (changePercent >= 0) {
        Console::setColor(TextColor::Green, bodyBg);
        priceStr << "(+" << std::fixed << std::setprecision(2) << changePercent << "%)";
    } else {
        Console::setColor(TextColor::Red, bodyBg);
        priceStr << "(" << std::fixed << std::setprecision(2) << changePercent << "%)";
    }

    Console::print(priceStr.str());
    currentY += 1;
    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("52-week Range: ");
    Console::setColor(TextColor::White, bodyBg);

    std::stringstream rangeStr;
    rangeStr << std::fixed << std::setprecision(2) << stock->getLowestPrice() << "$ - "
             << stock->getHighestPrice() << "$";
    Console::print(rangeStr.str());
    currentY += 1;

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
    currentY += 1;

    Console::setCursorPosition(x + 2, currentY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Dividends: ");

    const auto& dividendPolicy = company->getDividendPolicy();

    if (dividendPolicy.annualDividendRate > 0) {
        std::stringstream divStr;
        divStr << std::fixed << std::setprecision(2) << dividendPolicy.annualDividendRate << "$ (";

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

    Console::setCursorPosition(x, y + 7);
    Console::setColor(bodyFg, bodyBg);
    Console::drawHorizontalLine(x, y + 7, width);
}

void CompanyScreen::drawPriceChart() const {
    Console::setCursorPosition(x + 2, y + 8);
    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);

    priceChart.draw();

    Console::setCursorPosition(x, y + 19);
    Console::setColor(bodyFg, bodyBg);
    Console::drawHorizontalLine(x, y + 19, width);
}

void CompanyScreen::drawLatestNews() const {
    Console::setCursorPosition(x + 2, y + 20);
    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);
    Console::print("Latest News:");
    Console::setStyle(TextStyle::Regular);

    auto newsServicePtr = newsService.lock();
    if (!newsServicePtr || !company) {
        Console::setCursorPosition(x + 2, y + 21);
        Console::setColor(bodyFg, bodyBg);
        Console::print("No recent news about this company.");
        Console::drawHorizontalLine(x, y + 23, width);
        return;
    }

    const std::vector<News>& allNews = newsServicePtr->getNewsHistory();
    std::vector<News> companyNews;

    for (const auto& news : allNews) {
        if (news.getType() == NewsType::Corporate) {
            auto newsCompany = news.getTargetCompany().lock();

            if (newsCompany && newsCompany->getTicker() == company->getTicker()) {
                companyNews.push_back(news);
                continue;
            }

            if (news.getTitle().find(company->getName()) != std::string::npos ||
                news.getTitle().find(company->getTicker()) != std::string::npos ||
                news.getContent().find(company->getName()) != std::string::npos ||
                news.getContent().find(company->getTicker()) != std::string::npos) {
                companyNews.push_back(news);
            }
        }
        else if (news.getType() == NewsType::Sector && news.getTargetSector() == company->getSector()) {
            companyNews.push_back(news);
        }
    }

    std::sort(companyNews.begin(), companyNews.end(),
             [](const News& a, const News& b) {
                 return a.getPublishDate() > b.getPublishDate();
             });

    int currentY = y + 21;
    int newsCount = std::min(2, static_cast<int>(companyNews.size()));

    if (companyNews.empty()) {
        Console::setCursorPosition(x + 2, currentY);
        Console::setColor(bodyFg, bodyBg);
        Console::print("No recent news about this company.");
        Console::drawHorizontalLine(x, y + 23, width);
        return;
    }

    for (int i = 0; i < newsCount; ++i) {
        const News& news = companyNews[i];

        Console::setCursorPosition(x + 2, currentY + i);
        Console::setColor(TextColor::Cyan, bodyBg);
        Console::print(news.getPublishDate().toString() + ": ");
        Console::setColor(bodyFg, bodyBg);
        Console::print(news.getTitle());
    }

    Console::setCursorPosition(x, y + 23);
    Console::setColor(bodyFg, bodyBg);
    Console::drawHorizontalLine(x, y + 23, width);
}

void CompanyScreen::drawDescription() const {
    if (!company) {
        return;
    }

    std::string description = company->getDescription();
    if (description.empty()) {
        return;
    }

    if (y + 24 >= height - 4) {
        return;
    }
}

void CompanyScreen::drawActions() const {
    int actionY = height - 4;

    Console::setCursorPosition(x + 2, actionY);
    Console::setColor(bodyFg, bodyBg);
    Console::print("1. Buy Stocks");

    Console::setCursorPosition(x + 2, actionY + 1);
    Console::print("2. Sell Stocks");

    Console::setCursorPosition(x + 2, actionY + 2);
    Console::print("3. Return to Market");


    Console::setCursorPosition(x + 2, height - 1);
    Console::print("Choose action: ");

}

bool CompanyScreen::handleInput(int key) {
    switch (key) {
        case '1':
            buyStocks();
            return true;

        case '2':
            sellStocks();
            return true;

        case '3':
        case 27:
            close();
            return false;

        default:
            return Screen::handleInput(key);
    }
}

void CompanyScreen::buyStocks() {
    if (!company) {
        return;
    }

    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    int dialogY = y + 8;

    for (int i = 0; i < 11; ++i) {
        Console::setCursorPosition(x + 1, dialogY + i);
        Console::setColor(bodyFg, bodyBg);
        Console::print(std::string(width - 2, ' '));
    }

    Stock* stock = company->getStock();
    double price = stock->getCurrentPrice();
    double cash = playerPtr->getPortfolio()->getCashBalance();
    int maxStocks = static_cast<int>(cash / price);

    Console::setCursorPosition(x + 2, dialogY);
    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);
    Console::print("BUY STOCKS");
    Console::setStyle(TextStyle::Regular);

    Console::setCursorPosition(x + 2, dialogY + 2);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Current Price: ");
    Console::setColor(TextColor::Yellow, bodyBg);

    std::stringstream priceStr;
    priceStr << std::fixed << std::setprecision(2) << price << "$";
    Console::print(priceStr.str());

    Console::setCursorPosition(x + 2, dialogY + 3);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Available Cash: ");
    Console::setColor(TextColor::Green, bodyBg);

    std::stringstream cashStr;
    cashStr << std::fixed << std::setprecision(2) << cash << "$";
    Console::print(cashStr.str());

    Console::setCursorPosition(x + 2, dialogY + 4);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Max Stocks to Buy: ");
    Console::setColor(TextColor::Cyan, bodyBg);
    Console::print(std::to_string(maxStocks));

    if (maxStocks <= 0) {
        Console::setCursorPosition(x + 2, dialogY + 6);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("Insufficient funds to buy any stocks!");

        Console::setCursorPosition(x + 2, dialogY + 8);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Press any key to continue...");

        Console::readChar();
        draw();
        return;
    }

    Console::setCursorPosition(x + 2, dialogY + 6);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Enter quantity to buy: ");
    Console::setColor(TextColor::Yellow, bodyBg);

    std::string input = Console::readLine();
    int quantity = 0;

    try {
        quantity = std::stoi(input);
    } catch (...) {
        quantity = 0;
    }

    Console::setCursorPosition(x + 2, dialogY + 7);

    if (quantity <= 0 || quantity > maxStocks) {
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("Invalid quantity! Transaction canceled.");
    } else {
        bool success = playerPtr->buyStock(company, quantity);

        if (success) {
            double totalCost = price * quantity * 1.01;

            Console::setColor(TextColor::Green, bodyBg);
            Console::print("Successfully bought " + std::to_string(quantity) + " stocks for ");

            std::stringstream costStr;
            costStr << std::fixed << std::setprecision(2) << totalCost << "$";
            Console::print(costStr.str());
        } else {
            Console::setColor(TextColor::Red, bodyBg);
            Console::print("Transaction failed!");
        }
    }

    Console::setCursorPosition(x + 2, dialogY + 9);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Press any key to continue...");

    Console::readChar();
    draw();
}

void CompanyScreen::sellStocks() {
    if (!company) {
        return;
    }

    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    int dialogY = y + 8;

    for (int i = 0; i < 11; ++i) {
        Console::setCursorPosition(x + 1, dialogY + i);
        Console::setColor(bodyFg, bodyBg);
        Console::print(std::string(width - 2, ' '));
    }

    Stock* stock = company->getStock();
    double price = stock->getCurrentPrice();

    std::string ticker = company->getTicker();
    int ownedStocks = playerPtr->getPortfolio()->getPositionQuantity(ticker);

    Console::setCursorPosition(x + 2, dialogY);
    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);
    Console::print("SELL STOCKS");
    Console::setStyle(TextStyle::Regular);

    Console::setCursorPosition(x + 2, dialogY + 2);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Current Price: ");
    Console::setColor(TextColor::Yellow, bodyBg);

    std::stringstream priceStr;
    priceStr << std::fixed << std::setprecision(2) << price << "$";
    Console::print(priceStr.str());

    Console::setCursorPosition(x + 2, dialogY + 3);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Owned Stocks: ");
    Console::setColor(TextColor::Green, bodyBg);
    Console::print(std::to_string(ownedStocks));

    if (ownedStocks <= 0) {
        Console::setCursorPosition(x + 2, dialogY + 5);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("You don't own any stocks of this company!");

        Console::setCursorPosition(x + 2, dialogY + 7);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Press any key to continue...");

        Console::readChar();
        draw();
        return;
    }

    Console::setCursorPosition(x + 2, dialogY + 5);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Enter quantity to sell: ");
    Console::setColor(TextColor::Yellow, bodyBg);

    std::string input = Console::readLine();
    int quantity = 0;

    try {
        quantity = std::stoi(input);
    } catch (...) {
        quantity = 0;
    }

    Console::setCursorPosition(x + 2, dialogY + 6);

    if (quantity <= 0 || quantity > ownedStocks) {
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

    Console::setCursorPosition(x + 2, dialogY + 8);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Press any key to continue...");

    Console::readChar();
    draw();
}

}