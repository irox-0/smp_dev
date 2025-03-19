#include "NewsService.hpp"
#include <algorithm>
#include <sstream>

namespace StockMarketSimulator {

NewsService::NewsService()
    : currentDay(0),
      newsPerDay(2)
{
}

NewsService::NewsService(std::weak_ptr<Market> market)
    : market(market),
      currentDay(0),
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
        "Центральный банк %s процентную ставку",
        "Центральный банк принял решение %s базовую процентную ставку на %d базисных пунктов.",
        -0.03, 0.03,
        false, false
    ));
    
    newsTemplates.push_back(NewsTemplate(
        NewsType::Global,
        "Экономика показывает признаки %s",
        "Согласно последним экономическим показателям, экономика демонстрирует явные признаки %s.",
        -0.02, 0.02,
        false, false
    ));
    
    newsTemplates.push_back(NewsTemplate(
        NewsType::Sector,
        "Новые технологии в секторе",
        "В секторе появились новые технологии, которые могут существенно повлиять на рынок.",
        0.01, 0.04,
        false, false,
        Sector::Technology
    ));
    
    newsTemplates.push_back(NewsTemplate(
        NewsType::Sector,
        "Изменение цен на энергоносители",
        "Мировые цены на энергоносители демонстрируют значительные колебания.",
        -0.03, 0.03,
        false, false,
        Sector::Energy
    ));
    
    newsTemplates.push_back(NewsTemplate(
        NewsType::Corporate,
        "%s объявляет о новом продукте",
        "Компания %s объявила о выпуске нового продукта, который обещает изменить рынок.",
        0.02, 0.06,
        false, false
    ));
    
    newsTemplates.push_back(NewsTemplate(
        NewsType::Corporate,
        "%s публикует финансовый отчет",
        "Компания %s опубликовала квартальный финансовый отчет. %s",
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
    
    for (const auto& news : newsHistory) {
        if (news.getPublishDay() == day) {
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
    
    if (Random::getBool(0.7)) {
        News globalNews = generateRandomNews(NewsType::Global);
        generatedNews.push_back(globalNews);
    }
    
    if (Random::getBool(0.5) && generatedNews.size() < count) {
        News sectorNews = generateRandomNews(NewsType::Sector);
        generatedNews.push_back(sectorNews);
    }
    
    if (Random::getBool(0.7) && generatedNews.size() < count) {
        News corporateNews = generateRandomNews(NewsType::Corporate);
        generatedNews.push_back(corporateNews);
    }
    
    while (generatedNews.size() < count) {
        NewsType randomType = static_cast<NewsType>(Random::getInt(0, 2));
        News randomNews = generateRandomNews(randomType);
        generatedNews.push_back(randomNews);
    }
    
    for (auto& news : generatedNews) {
        news.setPublishDay(currentDay);
        newsHistory.push_back(news);
    }
    
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
    news.setPublishDay(currentDay);
    
    if (type == NewsType::Sector) {
        if (templ.targetSector != Sector::Unknown && currentDay % 2 != 0) {
            news.setTargetSector(templ.targetSector);
        } else {
            int sectorIndex = (currentDay + Random::getInt(0, 4)) % 5;
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
            return NewsTemplate(type, "Мировые рынки показывают изменения", 
                               "Мировые рынки демонстрируют значительные изменения.",
                               -0.02, 0.02);
        } else if (type == NewsType::Sector) {
            return NewsTemplate(type, "Изменения в секторе", 
                               "В секторе наблюдаются важные изменения.",
                               -0.01, 0.01, false, false, Sector::Technology);
        } else {
            return NewsTemplate(type, "Новости от компании", 
                               "Компания выпустила важное заявление.",
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
            
            if (result.find("ставк") != std::string::npos) {
                possibleWords = {"повысил", "снизил", "сохранил", "изменил"};
            } else if (result.find("экономик") != std::string::npos) {
                possibleWords = {"роста", "замедления", "стабилизации", "восстановления"};
            } else {
                possibleWords = {"изменения", "важные новости", "обновленные данные"};
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
            if (historyNews.getPublishDay() == newsItem.getPublishDay() &&
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

int NewsService::getCurrentDay() const {
    return currentDay;
}

void NewsService::setCurrentDay(int day) {
    if (day >= 0) {
        currentDay = day;
    }
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
    
    j["current_day"] = currentDay;
    j["news_per_day"] = newsPerDay;
    
    j["news_history"] = nlohmann::json::array();
    for (const auto& news : newsHistory) {
        j["news_history"].push_back(news.toJson());
    }
    
    return j;
}

NewsService NewsService::fromJson(const nlohmann::json& json, std::weak_ptr<Market> market) {
    NewsService service(market);
    
    service.currentDay = json["current_day"];
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

}