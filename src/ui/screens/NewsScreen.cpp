#include "NewsScreen.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace StockMarketSimulator {

NewsScreen::NewsScreen()
    : Screen("NEWS", ScreenType::News),
      currentFilter(NewsFilter::All),
      currentPage(0),
      newsPerPage(6)
{
    // Set size to match the mockup layout exactly
    setSize(46, 31);

}

void NewsScreen::setNewsService(std::weak_ptr<NewsService> newsService) {
    this->newsService = newsService;
}

void NewsScreen::initialize() {
    Screen::initialize();
    updateDisplayedNews();
}

void NewsScreen::update() {
    Screen::update();
    updateDisplayedNews();
}

void NewsScreen::updateDisplayedNews() {
    auto newsServicePtr = newsService.lock();
    if (!newsServicePtr) {
        displayedNews.clear();
        return;
    }

    const std::vector<News>& allNews = newsServicePtr->getNewsHistory();

    std::vector<News> filteredNews;

    for (const auto& news : allNews) {
        bool shouldInclude = false;

        switch (currentFilter) {
            case NewsFilter::All:
                shouldInclude = true;
                break;
            case NewsFilter::Global:
                shouldInclude = (news.getType() == NewsType::Global);
                break;
            case NewsFilter::Sector:
                shouldInclude = (news.getType() == NewsType::Sector);
                break;
            case NewsFilter::Corporate:
                shouldInclude = (news.getType() == NewsType::Corporate);
                break;
        }

        if (shouldInclude) {
            filteredNews.push_back(news);
        }
    }

    std::sort(filteredNews.begin(), filteredNews.end(),
             [](const News& a, const News& b) {
                 return a.getPublishDay() > b.getPublishDay();
             });

    int totalNews = static_cast<int>(filteredNews.size());
    int totalPages = std::max(1, (totalNews + newsPerPage - 1) / newsPerPage);

    if (currentPage >= totalPages) {
        currentPage = std::max(0, totalPages - 1);
    }

    int startIndex = currentPage * newsPerPage;
    int endIndex = std::min(startIndex + newsPerPage, totalNews);

    displayedNews.clear();
    for (int i = startIndex; i < endIndex; i++) {
        displayedNews.push_back(filteredNews[i]);
    }
}

void NewsScreen::drawContent() const {

    drawFilterInfo();

    drawNewsList();

    drawNavigationOptions();
}

void NewsScreen::drawFilterInfo() const {
    Console::setCursorPosition(x + 2, y + 2);
    Console::setColor(TextColor::White, bodyBg);

    std::string filterName;
    switch (currentFilter) {
        case NewsFilter::All:     filterName = "All News"; break;
        case NewsFilter::Global:  filterName = "Global News"; break;
        case NewsFilter::Sector:  filterName = "Sector News"; break;
        case NewsFilter::Corporate: filterName = "Company News"; break;
    }

    Console::print("Filter: " + filterName);

    auto marketPtr = market.lock();
    if (marketPtr) {
        Console::setCursorPosition(x + width - 22, y + 2);
        Console::print("Date: " + std::to_string(marketPtr->getCurrentDay()) + ".03.2023");
    }

    Console::setCursorPosition(x, y + 3);
    Console::drawHorizontalLine(x, y + 3, width);
}

void NewsScreen::drawNewsList() const {
    int newsY = y + 4;

    if (displayedNews.empty()) {
        Console::setCursorPosition(x + 2, newsY + 2);
        Console::setColor(bodyFg, bodyBg);
        Console::print("No news items to display.");
        return;
    }

    for (size_t i = 0; i < displayedNews.size(); i++) {
        const News& news = displayedNews[i];

        std::string typeStr;
        switch (news.getType()) {
            case NewsType::Global:
                typeStr = "[GLOBAL]";
                break;
            case NewsType::Sector:
                {
                    auto marketPtr = market.lock();
                    if (marketPtr) {
                        typeStr = "[SECTOR: " + Market::sectorToString(news.getTargetSector()) + "]";
                    } else {
                        typeStr = "[SECTOR]";
                    }
                }
                break;
            case NewsType::Corporate:
                {
                    auto company = news.getTargetCompany().lock();
                    if (company) {
                        typeStr = "[COMPANY: " + company->getName() + "]";
                    } else {
                        typeStr = "[COMPANY]";
                    }
                }
                break;
        }

        Console::setCursorPosition(x + 2, newsY);
        Console::setColor(TextColor::Cyan, bodyBg);
        Console::print(typeStr + " " + std::to_string(news.getPublishDay()) + ".03.2023");

        Console::setCursorPosition(x + 2, newsY + 1);
        Console::setColor(bodyFg, bodyBg);
        Console::print(news.getTitle());

        Console::setCursorPosition(x + 2, newsY + 2);
        Console::print(std::string(width - 4, ' '));

        newsY += 3;
    }
}

void NewsScreen::drawNavigationOptions() const {
    int bottomAreaStart = y + height - 10;

    Console::setCursorPosition(x, bottomAreaStart);
    Console::drawHorizontalLine(x, bottomAreaStart, width);

    int optionsY = bottomAreaStart + 1;

    Console::setCursorPosition(x + 2, optionsY);
    Console::setColor(bodyFg, bodyBg);
    Console::print(std::string(width - 4, ' '));

    Console::setCursorPosition(x + 2, optionsY);
    Console::print("To view the full text of the news");

    Console::setCursorPosition(x + 2, optionsY + 2);
    Console::print("enter its number (1-" + std::to_string(displayedNews.size()) + "):");

    Console::setCursorPosition(x + 2, optionsY + 3);
    Console::print(std::string(width - 4, ' '));

    Console::setCursorPosition(x + 2, optionsY + 4);
    Console::print("6. Change Filter");

    Console::setCursorPosition(x + 2, optionsY + 5);
    Console::print("7. Previous Page");

    Console::setCursorPosition(x + 2, optionsY + 6);
    Console::print("8. Next Page");

    Console::setCursorPosition(x + 2, optionsY + 7);
    Console::print("0. Return to Main Menu");
}

bool NewsScreen::handleInput(int key) {
    if (key >= '1' && key <= '5') {
        int newsIndex = key - '1';
        if (newsIndex >= 0 && newsIndex < static_cast<int>(displayedNews.size())) {
            displayNewsDetails(displayedNews[newsIndex]);
        }
        return true;
    }

    switch (key) {
        case '6':
            changeFilter();
            return true;

        case '7':
            previousPage();
            return true;

        case '8':
            nextPage();
            return true;

        case '0':
        case 27:
            close();
            return false;

        default:
            return Screen::handleInput(key);
    }
}

void NewsScreen::displayNewsDetails(const News& news) {
    std::string originalTitle = getTitle();

    setTitle("NEWS DETAILS");

    draw();

    for (int i = y + 2; i < y + height - 1; i++) {
        Console::setCursorPosition(x + 1, i);
        Console::setColor(bodyFg, bodyBg);
        Console::print(std::string(width - 2, ' '));
    }

    drawBorder();

    int contentY = y + 3;

    Console::setCursorPosition(x + 2, contentY);
    Console::setColor(TextColor::Cyan, bodyBg);

    std::string typeStr;
    switch (news.getType()) {
        case NewsType::Global:
            typeStr = "[GLOBAL]";
        break;
        case NewsType::Sector:
        {
            auto marketPtr = market.lock();
            if (marketPtr) {
                typeStr = "[SECTOR: " + Market::sectorToString(news.getTargetSector()) + "]";
            } else {
                typeStr = "[SECTOR]";
            }
        }
        break;
        case NewsType::Corporate:
        {
            auto company = news.getTargetCompany().lock();
            if (company) {
                typeStr = "[COMPANY: " + company->getName() + "]";
            } else {
                typeStr = "[COMPANY]";
            }
        }
        break;
    }

    Console::print(typeStr + " " + std::to_string(news.getPublishDay()) + ".03.2023");
    contentY += 2;

    Console::setCursorPosition(x + 2, contentY);
    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);
    Console::print(news.getTitle());
    Console::setStyle(TextStyle::Regular);
    contentY += 2;

    Console::setColor(bodyFg, bodyBg);
    std::string content = news.getContent();

    if (content.empty()) {
        content = "No detailed content available.";
    }

    int maxLineWidth = width - 4;
    std::istringstream iss(content);
    std::string word;
    std::string line;

    while (iss >> word) {
        if (line.empty()) {
            line = word;
        } else if (line.length() + word.length() + 1 <= maxLineWidth) {
            line += " " + word;
        } else {
            Console::setCursorPosition(x + 2, contentY++);
            Console::print(line);
            line = word;
        }
    }

    if (!line.empty()) {
        Console::setCursorPosition(x + 2, contentY++);
        Console::print(line);
    }

    contentY += 2;

    Console::setCursorPosition(x + 2, contentY);
    Console::setColor(TextColor::Yellow, bodyBg);
    Console::print("Market Impact: ");

    double impact = news.getImpact() * 100.0;
    if (impact > 0) {
        Console::setColor(TextColor::Green, bodyBg);
        Console::print("+" + std::to_string(impact) + "%");
    } else if (impact < 0) {
        Console::setColor(TextColor::Red, bodyBg);
        Console::print(std::to_string(impact) + "%");
    } else {
        Console::setColor(bodyFg, bodyBg);
        Console::print("Neutral");
    }

    contentY += 2;

    Console::setCursorPosition(x + 2, y + height - 3);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Press any key to return...");

    Console::readChar();

    setTitle(originalTitle);
    draw();
}

void NewsScreen::changeFilter() {
    std::vector<std::string> options = {
        "All News",
        "Global News",
        "Sector News",
        "Company News"
    };

    int bottomAreaStart = y + height - 10;
    int optionsY = bottomAreaStart + 1;

    for (int i = bottomAreaStart + 1; i < y + height - 2; i++) {
        Console::setCursorPosition(x + 1, i);
        Console::setColor(bodyFg, bodyBg);
        Console::print(std::string(width - 2, ' '));
    }

    Console::setCursorPosition(x + 2, optionsY);
    Console::setColor(TextColor::Cyan, bodyBg);
    Console::print("SELECT FILTER TYPE:");

    int menuX = x + 2;
    int menuY = optionsY + 2;

    int selected = static_cast<int>(currentFilter);
    bool running = true;

    while (running) {
        for (size_t i = 0; i < options.size(); i++) {
            Console::setCursorPosition(menuX, menuY + static_cast<int>(i));

            if (i == static_cast<size_t>(selected)) {
                Console::setColor(TextColor::Black, TextColor::White);
                Console::print("> " + options[i]);
            } else {
                Console::setColor(TextColor::White, bodyBg);
                Console::print("  " + options[i]);
            }

            int paddingLength = width - 4 - options[i].length() - 2;
            if (paddingLength > 0) {
                Console::print(std::string(paddingLength, ' '));
            }
        }

        Console::setCursorPosition(x + 2, menuY + static_cast<int>(options.size()) + 1);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Press Enter to select or Esc to cancel");

        Console::resetAttributes();

        char key = Console::readChar();

        switch (key) {
            case static_cast<char>(Key::ArrowUp):
                selected = (selected > 0) ? selected - 1 : static_cast<int>(options.size()) - 1;
                break;

            case static_cast<char>(Key::ArrowDown):
                selected = (selected < static_cast<int>(options.size()) - 1) ? selected + 1 : 0;
                break;

            case static_cast<char>(Key::Enter):
                running = false;
                break;

            case static_cast<char>(Key::Escape):
                selected = -1;
                running = false;
                break;

            default:
                if (key >= '1' && key <= '4') {
                    selected = key - '1';
                    running = false;
                }
                break;
        }
    }

    if (selected >= 0) {
        switch (selected) {
            case 0: setCurrentFilter(NewsFilter::All); break;
            case 1: setCurrentFilter(NewsFilter::Global); break;
            case 2: setCurrentFilter(NewsFilter::Sector); break;
            case 3: setCurrentFilter(NewsFilter::Corporate); break;
        }
    }

    draw();
}

void NewsScreen::previousPage() {
    if (currentPage > 0) {
        currentPage--;
        updateDisplayedNews();
        draw();
    }
}

void NewsScreen::nextPage() {
    int totalPages = getTotalPages();
    if (currentPage < totalPages - 1) {
        currentPage++;
        updateDisplayedNews();
        draw();
    }
}

NewsFilter NewsScreen::getCurrentFilter() const {
    return currentFilter;
}

void NewsScreen::setCurrentFilter(NewsFilter filter) {
    currentFilter = filter;
    currentPage = 0;
    updateDisplayedNews();
}

int NewsScreen::getCurrentPage() const {
    return currentPage;
}

void NewsScreen::setCurrentPage(int page) {
    if (page >= 0) {
        currentPage = page;
        updateDisplayedNews();
    }
}

int NewsScreen::getNewsPerPage() const {
    return newsPerPage;
}

void NewsScreen::setNewsPerPage(int count) {
    if (count > 0) {
        newsPerPage = count;
        updateDisplayedNews();
    }
}

int NewsScreen::getTotalPages() const {
    auto newsServicePtr = newsService.lock();
    if (!newsServicePtr) {
        return 1;
    }

    const std::vector<News>& allNews = newsServicePtr->getNewsHistory();

    int filteredCount = 0;
    for (const auto& news : allNews) {
        bool shouldInclude = false;

        switch (currentFilter) {
            case NewsFilter::All:
                shouldInclude = true;
                break;
            case NewsFilter::Global:
                shouldInclude = (news.getType() == NewsType::Global);
                break;
            case NewsFilter::Sector:
                shouldInclude = (news.getType() == NewsType::Sector);
                break;
            case NewsFilter::Corporate:
                shouldInclude = (news.getType() == NewsType::Corporate);
                break;
        }

        if (shouldInclude) {
            filteredCount++;
        }
    }

    return std::max(1, (filteredCount + newsPerPage - 1) / newsPerPage);
}

}