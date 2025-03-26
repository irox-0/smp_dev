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

    // Filter news by type
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

    // Sort news by publish day (newest first)
    std::sort(filteredNews.begin(), filteredNews.end(),
             [](const News& a, const News& b) {
                 return a.getPublishDay() > b.getPublishDay();
             });

    // Calculate page bounds
    int totalNews = static_cast<int>(filteredNews.size());
    int totalPages = std::max(1, (totalNews + newsPerPage - 1) / newsPerPage);

    // Adjust current page if needed
    if (currentPage >= totalPages) {
        currentPage = std::max(0, totalPages - 1);
    }

    // Get subset of news for current page
    int startIndex = currentPage * newsPerPage;
    int endIndex = std::min(startIndex + newsPerPage, totalNews);

    displayedNews.clear();
    for (int i = startIndex; i < endIndex; i++) {
        displayedNews.push_back(filteredNews[i]);
    }
}

void NewsScreen::drawContent() const {
    // Note: The base Screen class already draws the border and title
    // We just need to fill in the content

    // Draw filter bar
    drawFilterInfo();

    // Draw news list
    drawNewsList();

    // Draw navigation options
    drawNavigationOptions();
}

void NewsScreen::drawFilterInfo() const {
    // Fill in the filter info area
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

    // Display current date
    auto marketPtr = market.lock();
    if (marketPtr) {
        Console::setCursorPosition(x + width - 22, y + 2);
        Console::print("Date: " + std::to_string(marketPtr->getCurrentDay()) + ".03.2023");
    }

    // Draw separator line
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

        // Format news type and date
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

        // Display type and date
        Console::setCursorPosition(x + 2, newsY);
        Console::setColor(TextColor::Cyan, bodyBg);
        Console::print(typeStr + " " + std::to_string(news.getPublishDay()) + ".03.2023");

        // Display title/content
        Console::setCursorPosition(x + 2, newsY + 1);
        Console::setColor(bodyFg, bodyBg);
        Console::print(news.getTitle());

        // Add empty line after each news item
        Console::setCursorPosition(x + 2, newsY + 2);
        // Fill the line with spaces to ensure a clean empty line
        Console::print(std::string(width - 4, ' '));

        newsY += 3;
    }
}

void NewsScreen::drawNavigationOptions() const {
    // Calculate positions to ensure we stay within borders
    int bottomAreaStart = y + height - 10;

    // Draw a horizontal separator line
    Console::setCursorPosition(x, bottomAreaStart);
    Console::drawHorizontalLine(x, bottomAreaStart, width);

    int optionsY = bottomAreaStart + 1;

    // Empty line
    Console::setCursorPosition(x + 2, optionsY);
    Console::setColor(bodyFg, bodyBg);
    Console::print(std::string(width - 4, ' '));

    // Instructions
    Console::setCursorPosition(x + 2, optionsY);
    Console::print("To view the full text of the news");

    Console::setCursorPosition(x + 2, optionsY + 2);
    Console::print("enter its number (1-" + std::to_string(displayedNews.size()) + "):");

    // Empty line
    Console::setCursorPosition(x + 2, optionsY + 3);
    Console::print(std::string(width - 4, ' '));

    // Menu options
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
    // Handle numeric keys for news selection
    if (key >= '1' && key <= '5') {
        int newsIndex = key - '1';
        if (newsIndex >= 0 && newsIndex < static_cast<int>(displayedNews.size())) {
            displayNewsDetails(displayedNews[newsIndex]);
        }
        return true;
    }

    // Handle other options
    switch (key) {
        case '6': // Change filter
            changeFilter();
            return true;

        case '7': // Previous page
            previousPage();
            return true;

        case '8': // Next page
            nextPage();
            return true;

        case '0': // Return to main menu
        case 27:  // ESC key
            close();
            return false;

        default:
            return Screen::handleInput(key);
    }
}

void NewsScreen::displayNewsDetails(const News& news) {
    // Store original title
    std::string originalTitle = getTitle();

    // Update screen for news details
    setTitle("NEWS DETAILS");

    // Draw our screen with the new title
    draw();

    // Clear the screen area for content
    for (int i = y + 2; i < y + height - 1; i++) {
        Console::setCursorPosition(x + 1, i);
        Console::setColor(bodyFg, bodyBg);
        Console::print(std::string(width - 2, ' '));
    }

    // Redraw box outline to ensure it's clean
    drawBorder();

    int contentY = y + 3;

    // Display news type and date
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

    // Display title
    Console::setCursorPosition(x + 2, contentY);
    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);
    Console::print(news.getTitle());
    Console::setStyle(TextStyle::Regular);
    contentY += 2;

    // Display content with word wrapping
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

    // Display market impact
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

    // Press any key to continue
    Console::setCursorPosition(x + 2, y + height - 3);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Press any key to return...");

    // Wait for key press
    Console::readChar();

    // Restore original title and redraw main screen
    setTitle(originalTitle);
    draw();
}

void NewsScreen::changeFilter() {
    // Define filter options
    std::vector<std::string> options = {
        "All News",
        "Global News",
        "Sector News",
        "Company News"
    };

    // Calculate the position for the navigation area
    int bottomAreaStart = y + height - 10;
    int optionsY = bottomAreaStart + 1;

    // Clear the navigation options area first
    for (int i = bottomAreaStart + 1; i < y + height - 2; i++) {
        Console::setCursorPosition(x + 1, i);
        Console::setColor(bodyFg, bodyBg);
        Console::print(std::string(width - 2, ' '));
    }

    // Draw a title for the filter selection
    Console::setCursorPosition(x + 2, optionsY);
    Console::setColor(TextColor::Cyan, bodyBg);
    Console::print("SELECT FILTER TYPE:");

    // Position the menu in the navigation options area
    int menuX = x + 2;
    int menuY = optionsY + 2;

    int selected = static_cast<int>(currentFilter);
    bool running = true;

    while (running) {
        // Draw the menu options
        for (size_t i = 0; i < options.size(); i++) {
            Console::setCursorPosition(menuX, menuY + static_cast<int>(i));

            if (i == static_cast<size_t>(selected)) {
                Console::setColor(TextColor::Black, TextColor::White);
                Console::print("> " + options[i]);
            } else {
                Console::setColor(TextColor::White, bodyBg);
                Console::print("  " + options[i]);
            }

            // Clear the rest of the line to prevent artifacts
            int paddingLength = width - 4 - options[i].length() - 2; // -4 for borders, -2 for "> "
            if (paddingLength > 0) {
                Console::print(std::string(paddingLength, ' '));
            }
        }

        // Add instructions at the bottom
        Console::setCursorPosition(x + 2, menuY + static_cast<int>(options.size()) + 1);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Press Enter to select or Esc to cancel");

        Console::resetAttributes();

        // Handle key input
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
                if (key >= '1' && key <= '4') { // 1-4 for menu options
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

    // Completely redraw screen to restore navigation options
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

    // Count news that match current filter
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