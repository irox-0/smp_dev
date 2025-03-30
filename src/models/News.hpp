#pragma once

#include <string>
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>
#include "Company.hpp"
#include "../utils/Date.hpp"

namespace StockMarketSimulator {

enum class NewsType {
    Global,
    Sector,
    Corporate
};

class News {
private:
    NewsType type;
    std::string title;
    std::string content;
    double impact;
    Date publishDate;
    Sector targetSector;
    std::weak_ptr<Company> targetCompany;
    bool processed;

public:
    News();
    News(NewsType type, const std::string& title, const std::string& content,
         double impact, const Date& publishDate);
    News(NewsType type, const std::string& title, const std::string& content,
         double impact, const Date& publishDate, Sector targetSector);
    News(NewsType type, const std::string& title, const std::string& content,
         double impact, const Date& publishDate, std::weak_ptr<Company> targetCompany);

    NewsType getType() const;
    std::string getTitle() const;
    std::string getContent() const;
    double getImpact() const;
    Date getPublishDate() const;
    Sector getTargetSector() const;
    std::weak_ptr<Company> getTargetCompany() const;
    bool isProcessed() const;

    void setType(NewsType type);
    void setTitle(const std::string& title);
    void setContent(const std::string& content);
    void setImpact(double impact);
    void setPublishDate(const Date& date);
    void setTargetSector(Sector sector);
    void setTargetCompany(std::weak_ptr<Company> company);
    void setProcessed(bool processed);

    bool shouldAffectMarket() const;
    bool shouldAffectSector(Sector sector) const;
    bool shouldAffectCompany(const std::shared_ptr<Company>& company) const;
    static std::string newsTypeToString(NewsType type);
    static NewsType newsTypeFromString(const std::string& typeStr);

    nlohmann::json toJson() const;
    static News fromJson(const nlohmann::json& json, 
                         const std::vector<std::shared_ptr<Company>>& companies = {});
};

struct NewsTemplate {
    NewsType type;
    std::string titleTemplate;
    std::string contentTemplate;
    double minImpact;
    double maxImpact;
    bool requiresPositiveMarket;
    bool requiresNegativeMarket;
    Sector targetSector;
    
    NewsTemplate();
    NewsTemplate(NewsType type, const std::string& titleTemplate, 
                 const std::string& contentTemplate,
                 double minImpact, double maxImpact,
                 bool requiresPositiveMarket = false,
                 bool requiresNegativeMarket = false,
                 Sector targetSector = Sector::Unknown);
                 
    nlohmann::json toJson() const;
    static NewsTemplate fromJson(const nlohmann::json& json);
};

}