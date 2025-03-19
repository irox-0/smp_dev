#include "News.hpp"
#include "../core/Market.hpp"
#include <algorithm>

namespace StockMarketSimulator {

News::News()
    : type(NewsType::Global),
      title(""),
      content(""),
      impact(0.0),
      publishDay(0),
      targetSector(Sector::Unknown),
      processed(false)
{
}

News::News(NewsType type, const std::string& title, const std::string& content,
           double impact, int publishDay)
    : type(type),
      title(title),
      content(content),
      impact(impact),
      publishDay(publishDay),
      targetSector(Sector::Unknown),
      processed(false)
{
}

News::News(NewsType type, const std::string& title, const std::string& content,
           double impact, int publishDay, Sector targetSector)
    : type(type),
      title(title),
      content(content),
      impact(impact),
      publishDay(publishDay),
      targetSector(targetSector),
      processed(false)
{
}

News::News(NewsType type, const std::string& title, const std::string& content,
           double impact, int publishDay, std::weak_ptr<Company> targetCompany)
    : type(type),
      title(title),
      content(content),
      impact(impact),
      publishDay(publishDay),
      targetSector(Sector::Unknown),
      targetCompany(targetCompany),
      processed(false)
{
    auto company = targetCompany.lock();
    if (company) {
        targetSector = company->getSector();
    }
}

NewsType News::getType() const {
    return type;
}

std::string News::getTitle() const {
    return title;
}

std::string News::getContent() const {
    return content;
}

double News::getImpact() const {
    return impact;
}

int News::getPublishDay() const {
    return publishDay;
}

Sector News::getTargetSector() const {
    return targetSector;
}

std::weak_ptr<Company> News::getTargetCompany() const {
    return targetCompany;
}

bool News::isProcessed() const {
    return processed;
}

void News::setType(NewsType type) {
    this->type = type;
}

void News::setTitle(const std::string& title) {
    this->title = title;
}

void News::setContent(const std::string& content) {
    this->content = content;
}

void News::setImpact(double impact) {
    this->impact = impact;
}

void News::setPublishDay(int day) {
    if (day >= 0) {
        this->publishDay = day;
    }
}

void News::setTargetSector(Sector sector) {
    this->targetSector = sector;
}

void News::setTargetCompany(std::weak_ptr<Company> company) {
    this->targetCompany = company;

    auto companyPtr = company.lock();
    if (companyPtr) {
        targetSector = companyPtr->getSector();
    }
}

void News::setProcessed(bool processed) {
    this->processed = processed;
}

bool News::shouldAffectMarket() const {
    return type == NewsType::Global;
}

bool News::shouldAffectSector(Sector sector) const {
    return (type == NewsType::Sector && targetSector == sector) || type == NewsType::Global;
}

bool News::shouldAffectCompany(const std::shared_ptr<Company>& company) const {
    if (!company) {
        return false;
    }

    if (type == NewsType::Global) {
        return true;
    }

    if (type == NewsType::Sector) {
        return company->getSector() == targetSector;
    }

    if (type == NewsType::Corporate) {
        auto targetComp = targetCompany.lock();
        if (targetComp) {
            return targetComp->getTicker() == company->getTicker();
        }
    }

    return false;
}
std::string News::newsTypeToString(NewsType type) {
    switch (type) {
        case NewsType::Global: return "Global";
        case NewsType::Sector: return "Sector";
        case NewsType::Corporate: return "Corporate";
        default: return "Unknown";
    }
}

NewsType News::newsTypeFromString(const std::string& typeStr) {
    if (typeStr == "Global") return NewsType::Global;
    if (typeStr == "Sector") return NewsType::Sector;
    if (typeStr == "Corporate") return NewsType::Corporate;

    return NewsType::Global;
}

nlohmann::json News::toJson() const {
    nlohmann::json j;

    j["type"] = newsTypeToString(type);
    j["title"] = title;
    j["content"] = content;
    j["impact"] = impact;
    j["publish_day"] = publishDay;
    j["processed"] = processed;

    j["target_sector"] = Market::sectorToString(targetSector);

    auto company = targetCompany.lock();
    if (company) {
        j["target_company_ticker"] = company->getTicker();
    } else {
        j["target_company_ticker"] = "";
    }

    return j;
}

News News::fromJson(const nlohmann::json& json, const std::vector<std::shared_ptr<Company>>& companies) {
    News news;

    news.type = newsTypeFromString(json["type"]);
    news.title = json["title"];
    news.content = json["content"];
    news.impact = json["impact"];
    news.publishDay = json["publish_day"];
    news.processed = json["processed"];

    news.targetSector = Market::sectorFromString(json["target_sector"]);

    std::string targetCompanyTicker = json["target_company_ticker"];
    if (!targetCompanyTicker.empty() && !companies.empty()) {
        for (const auto& company : companies) {
            if (company->getTicker() == targetCompanyTicker) {
                news.targetCompany = company;
                break;
            }
        }
    }

    return news;
}

NewsTemplate::NewsTemplate()
    : type(NewsType::Global),
      titleTemplate(""),
      contentTemplate(""),
      minImpact(0.0),
      maxImpact(0.0),
      requiresPositiveMarket(false),
      requiresNegativeMarket(false),
      targetSector(Sector::Unknown)
{
}

NewsTemplate::NewsTemplate(NewsType type, const std::string& titleTemplate,
                         const std::string& contentTemplate,
                         double minImpact, double maxImpact,
                         bool requiresPositiveMarket,
                         bool requiresNegativeMarket,
                         Sector targetSector)
    : type(type),
      titleTemplate(titleTemplate),
      contentTemplate(contentTemplate),
      minImpact(minImpact),
      maxImpact(maxImpact),
      requiresPositiveMarket(requiresPositiveMarket),
      requiresNegativeMarket(requiresNegativeMarket),
      targetSector(targetSector)
{
}

nlohmann::json NewsTemplate::toJson() const {
    nlohmann::json j;

    j["type"] = News::newsTypeToString(type);
    j["title_template"] = titleTemplate;
    j["content_template"] = contentTemplate;
    j["min_impact"] = minImpact;
    j["max_impact"] = maxImpact;
    j["requires_positive_market"] = requiresPositiveMarket;
    j["requires_negative_market"] = requiresNegativeMarket;
    j["target_sector"] = Market::sectorToString(targetSector);

    return j;
}

NewsTemplate NewsTemplate::fromJson(const nlohmann::json& json) {
    NewsTemplate tmpl;

    tmpl.type = News::newsTypeFromString(json["type"]);
    tmpl.titleTemplate = json["title_template"];
    tmpl.contentTemplate = json["content_template"];
    tmpl.minImpact = json["min_impact"];
    tmpl.maxImpact = json["max_impact"];

    if (json.contains("requires_positive_market")) {
        tmpl.requiresPositiveMarket = json["requires_positive_market"];
    }

    if (json.contains("requires_negative_market")) {
        tmpl.requiresNegativeMarket = json["requires_negative_market"];
    }

    if (json.contains("target_sector")) {
        tmpl.targetSector = Market::sectorFromString(json["target_sector"]);
    }
    
    return tmpl;
}

}