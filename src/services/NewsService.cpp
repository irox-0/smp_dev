#include "NewsService.hpp"
#include <algorithm>
#include <sstream>

namespace StockMarketSimulator {

NewsService::NewsService()
    : currentDate(1, 3, 2023),
      newsPerDay(2)
{
}

NewsService::NewsService(std::weak_ptr<Market> market)
    : market(market),
      currentDate(1, 3, 2023),
      newsPerDay(2)
{
}

void NewsService::initialize(const std::string& templatesPath) {
    loadNewsTemplates(templatesPath);
}

void NewsService::loadNewsTemplates(const std::string& filePath) {
    try {
        if (FileIO::fileExists(filePath)) {
            nlohmann::json json = FileIO::readJsonFile(filePath);

            newsTemplates.clear();
            categoryTemplates.clear();

            for (const auto& templateJson : json) {
                NewsTemplate newsTemplate = NewsTemplate::fromJson(templateJson);
                newsTemplates.push_back(newsTemplate);

                categoryTemplates[newsTemplate.type].push_back(newsTemplate);
            }
        } else {
            createDefaultTemplates();
        }
    } catch (const std::exception& e) {
        createDefaultTemplates();
    }
}

void NewsService::createDefaultTemplates() {
    newsTemplates.push_back(NewsTemplate(
        NewsType::Global,
        "Central bank %s interest rate",
        "The Central Bank has decided  %s base interest rate by  %d basis points.",
        -0.03, 0.03,
        false, false
    ));

    newsTemplates.push_back(NewsTemplate(
        NewsType::Global,
        "The economy is showing signs %s",
        "According to the latest economic indicators, the economy is showing clear signs %s.",
        -0.02, 0.02,
        false, false
    ));

    newsTemplates.push_back(NewsTemplate(
        NewsType::Sector,
        "New technologies in the sector",
        "New technologies have emerged in the sector that can significantly affect the market.",
        0.01, 0.04,
        false, false,
        Sector::Technology
    ));

    newsTemplates.push_back(NewsTemplate(
        NewsType::Sector,
        "Changes in energy prices",
        "Global energy prices show significant fluctuations.",
        -0.03, 0.03,
        false, false,
        Sector::Energy
    ));

    newsTemplates.push_back(NewsTemplate(
        NewsType::Corporate,
        "%s announces a new product",
        "Company %s announced the release of a new product that promises to change the market.",
        0.02, 0.06,
        false, false
    ));

    newsTemplates.push_back(NewsTemplate(
        NewsType::Corporate,
        "%s publishes a financial report",
        "Company %s published the quarterly financial report. %s",
        -0.04, 0.04,
        false, false
    ));

    for (const auto& tmpl : newsTemplates) {
        categoryTemplates[tmpl.type].push_back(tmpl);
    }
}

void NewsService::setMarket(std::weak_ptr<Market> market) {
    this->market = market;
}

const std::vector<News>& NewsService::getNewsHistory() const {
    return newsHistory;
}

std::vector<News> NewsService::getNewsByDay(int day) const {
    std::vector<News> result;

    Date date = Date::fromDayNumber(day);

    for (const auto& news : newsHistory) {
        if (news.getPublishDate() == date) {
            result.push_back(news);
        }
    }

    return result;
}

std::vector<News> NewsService::getLatestNews(int count) const {
    std::vector<News> result;

    int size = static_cast<int>(newsHistory.size());
    int startIdx = std::max(0, size - count);

    for (int i = startIdx; i < size; ++i) {
        result.push_back(newsHistory[i]);
    }

    return result;
}

std::vector<News> NewsService::generateDailyNews(int newsCount) {
    auto marketPtr = market.lock();
    if (!marketPtr) {
        return {};
    }

    int count = (newsCount > 0) ? newsCount : newsPerDay;
    std::vector<News> generatedNews;

    MarketTrend currentTrend = marketPtr->getCurrentTrend();

    // Try to generate news of each type, checking for duplicates
    int maxAttempts = count * 3; // Allow multiple attempts to find unique news
    int attempts = 0;

    // Try to generate global news
    if (Random::getBool(0.7) && generatedNews.size() < count) {
        News globalNews = generateRandomNews(NewsType::Global);
        if (!isDuplicateNews(globalNews)) {
            generatedNews.push_back(globalNews);
        }
    }

    // Try to generate sector news
    if (Random::getBool(0.5) && generatedNews.size() < count) {
        News sectorNews = generateRandomNews(NewsType::Sector);
        if (!isDuplicateNews(sectorNews)) {
            generatedNews.push_back(sectorNews);
        }
    }

    // Try to generate corporate news
    if (Random::getBool(0.7) && generatedNews.size() < count) {
        News corporateNews = generateRandomNews(NewsType::Corporate);
        if (!isDuplicateNews(corporateNews)) {
            generatedNews.push_back(corporateNews);
        }
    }

    // Keep generating random news until we reach the desired count or max attempts
    while (generatedNews.size() < count && attempts < maxAttempts) {
        NewsType randomType = static_cast<NewsType>(Random::getInt(0, 2));
        News randomNews = generateRandomNews(randomType);

        // Only add if it's not a duplicate
        if (!isDuplicateNews(randomNews)) {
            generatedNews.push_back(randomNews);
        }

        attempts++;
    }

    // Set publish date and add to history
    for (auto& news : generatedNews) {
        news.setPublishDate(currentDate);
        newsHistory.push_back(news);
    }

    // Maintain a reasonable history size
    const size_t MAX_HISTORY_SIZE = 1000;
    if (newsHistory.size() > MAX_HISTORY_SIZE) {
        newsHistory.erase(newsHistory.begin(), newsHistory.begin() + (newsHistory.size() - MAX_HISTORY_SIZE));
    }

    return generatedNews;
}
News NewsService::generateRandomNews(NewsType type) {
    auto marketPtr = market.lock();
    if (!marketPtr) {
        return News();
    }

    MarketTrend trend = marketPtr->getCurrentTrend();
    NewsTemplate templ = selectNewsTemplate(type, trend);

    double impact = Random::getDouble(templ.minImpact, templ.maxImpact);

    if (trend == MarketTrend::Bullish && impact < 0) {
        impact *= 0.5;
    } else if (trend == MarketTrend::Bullish && impact > 0) {
        impact *= 1.2;
    } else if (trend == MarketTrend::Bearish && impact > 0) {
        impact *= 0.5;
    } else if (trend == MarketTrend::Bearish && impact < 0) {
        impact *= 1.2;
    }

    std::shared_ptr<Company> targetCompany;

    if (type == NewsType::Corporate) {
        const auto& companies = marketPtr->getCompanies();
        if (!companies.empty()) {
            targetCompany = companies[Random::getIndex(companies.size())];
        }
    }

    std::string title = processTemplate(templ.titleTemplate, targetCompany);
    std::string content = processTemplate(templ.contentTemplate, targetCompany);

    News news;
    news.setType(type);
    news.setTitle(title);
    news.setContent(content);
    news.setImpact(impact);
    news.setPublishDate(currentDate);

    if (type == NewsType::Sector) {
        if (templ.targetSector != Sector::Unknown && currentDate.getDay() % 2 != 0) {
            news.setTargetSector(templ.targetSector);
        } else {
            int sectorIndex = (currentDate.getDay() + Random::getInt(0, 4)) % 5;
            Sector randomSector = static_cast<Sector>(sectorIndex);
            news.setTargetSector(randomSector);
        }
    } else if (type == NewsType::Corporate && targetCompany) {
        news.setTargetCompany(targetCompany);
    }

    return news;
}

NewsTemplate NewsService::selectNewsTemplate(NewsType type, MarketTrend trend) {
    auto& templates = categoryTemplates[type];

    if (templates.empty()) {
        if (type == NewsType::Global) {
            return NewsTemplate(type, "Global markets show changes",
                               "Global markets are showing significant changes.",
                               -0.02, 0.02);
        } else if (type == NewsType::Sector) {
            return NewsTemplate(type, "Changes in the sector",
                               "Important changes are being observed in the sector.",
                               -0.01, 0.01, false, false, Sector::Technology);
        } else {
            return NewsTemplate(type, "News from the company",
                               "The company has released an important statement.",
                               -0.03, 0.03);
        }
    }

    std::vector<NewsTemplate> filteredTemplates;
    bool isBullish = (trend == MarketTrend::Bullish);
    bool isBearish = (trend == MarketTrend::Bearish);

    for (const auto& tmpl : templates) {
        if ((isBullish && tmpl.requiresPositiveMarket) ||
            (isBearish && tmpl.requiresNegativeMarket) ||
            (!tmpl.requiresPositiveMarket && !tmpl.requiresNegativeMarket)) {
            filteredTemplates.push_back(tmpl);
        }
    }

    if (filteredTemplates.empty()) {
        return templates[Random::getIndex(templates.size())];
    } else {
        return filteredTemplates[Random::getIndex(filteredTemplates.size())];
    }
}

std::string NewsService::processTemplate(const std::string& templ,
                                        const std::shared_ptr<Company>& company) {
    std::string result = templ;

    if (company) {
        size_t pos = result.find("%s");
        while (pos != std::string::npos) {
            result.replace(pos, 2, company->getName());
            pos = result.find("%s", pos + company->getName().length());
        }
    } else {
        size_t pos = result.find("%s");
        while (pos != std::string::npos) {
            std::vector<std::string> possibleWords;

            if (result.find("rate") != std::string::npos) {
                possibleWords = {"increased", "decreased", "saved", "changed"};
            } else if (result.find("economic") != std::string::npos) {
                possibleWords = {"growth", "deceleration", "stabilization", "recovery"};
            } else {
                possibleWords = {"changes", "important news", "updated data"};
            }

            std::string replacement = possibleWords[Random::getIndex(possibleWords.size())];
            result.replace(pos, 2, replacement);

            pos = result.find("%s", pos + replacement.length());
        }
    }

    size_t pos = result.find("%d");
    while (pos != std::string::npos) {
        int randomNumber = Random::getInt(5, 50);
        std::string replacement = std::to_string(randomNumber);
        result.replace(pos, 2, replacement);

        pos = result.find("%d", pos + replacement.length());
    }

    return result;
}

void NewsService::applyNewsEffects(const std::vector<News>& news) {
    auto marketPtr = market.lock();
    if (!marketPtr) {
        return;
    }

    for (auto& newsItem : const_cast<std::vector<News>&>(news)) {
        if (newsItem.isProcessed()) {
            continue;
        }

        double impact = newsItem.getImpact();

        if (newsItem.shouldAffectMarket()) {
            marketPtr->triggerEconomicEvent(impact);
        } else if (newsItem.getType() == NewsType::Sector) {
            Sector targetSector = newsItem.getTargetSector();
            for (const auto& company : marketPtr->getCompaniesBySector(targetSector)) {
                company->processNewsImpact(impact);
            }
        } else if (newsItem.getType() == NewsType::Corporate) {
            auto targetCompany = newsItem.getTargetCompany().lock();
            if (targetCompany) {
                targetCompany->processNewsImpact(impact);
            }
        }

        newsItem.setProcessed(true);

        for (auto& historyNews : newsHistory) {
            if (historyNews.getPublishDate() == newsItem.getPublishDate() &&
                historyNews.getTitle() == newsItem.getTitle() &&
                historyNews.getContent() == newsItem.getContent()) {
                historyNews.setProcessed(true);
                break;
                }
        }
    }
}

void NewsService::addCustomNews(const News& news) {
    newsHistory.push_back(news);
}

Date NewsService::getCurrentDate() const {
    return currentDate;
}

void NewsService::setCurrentDate(const Date& date) {
    this->currentDate = date;
}

int NewsService::getNewsPerDay() const {
    return newsPerDay;
}

void NewsService::setNewsPerDay(int count) {
    if (count > 0) {
        newsPerDay = count;
    }
}

nlohmann::json NewsService::toJson() const {
    nlohmann::json j;

    j["current_date"] = currentDate.toJson();
    j["news_per_day"] = newsPerDay;

    j["news_history"] = nlohmann::json::array();
    for (const auto& news : newsHistory) {
        j["news_history"].push_back(news.toJson());
    }

    return j;
}

NewsService NewsService::fromJson(const nlohmann::json& json, std::weak_ptr<Market> market) {
    NewsService service(market);

    if (json.contains("current_date")) {
        service.currentDate = Date::fromJson(json["current_date"]);
    } else if (json.contains("current_day")) {
        int currentDay = json["current_day"];
        service.currentDate = Date::fromDayNumber(currentDay);
    } else {
        service.currentDate = Date(1, 3, 2023);
    }

    service.newsPerDay = json["news_per_day"];

    auto marketPtr = market.lock();
    if (marketPtr) {
        const auto& companies = marketPtr->getCompanies();

        for (const auto& newsJson : json["news_history"]) {
            News news = News::fromJson(newsJson, companies);
            service.newsHistory.push_back(news);
        }
    }
    
    service.initialize();
    
    return service;
}
bool NewsService::isDuplicateNews(const News& news) const {
    // Check the recent news history (last 10 items is usually sufficient)
    size_t checkCount = std::min(newsHistory.size(), static_cast<size_t>(10));

    for (size_t i = newsHistory.size() - checkCount; i < newsHistory.size(); i++) {
        const News& existingNews = newsHistory[i];

        // If date, title, and type match, consider it a duplicate
        if (existingNews.getPublishDate() == news.getPublishDate() &&
            existingNews.getTitle() == news.getTitle() &&
            existingNews.getType() == news.getType()) {

            // For company news, check if they target the same company
            if (news.getType() == NewsType::Corporate) {
                auto existingCompany = existingNews.getTargetCompany().lock();
                auto newCompany = news.getTargetCompany().lock();

                // If both have valid companies and they are the same
                if (existingCompany && newCompany &&
                    existingCompany->getTicker() == newCompany->getTicker()) {
                    return true;
                    }
            }
            // For sector news, check if they target the same sector
            else if (news.getType() == NewsType::Sector &&
                     existingNews.getTargetSector() == news.getTargetSector()) {
                return true;
                     }
            // For global news, title and date matching is sufficient
            else if (news.getType() == NewsType::Global) {
                return true;
            }
            }
    }

    return false;
}
}