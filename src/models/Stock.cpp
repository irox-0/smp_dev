#include "Stock.hpp"
#include "Company.hpp"
#include "../utils/Random.hpp"
#include <algorithm>
#include <cmath>

namespace StockMarketSimulator {

Stock::Stock()
    : currentPrice(0.0),
      lastUpdateTime(std::time(nullptr)),
      lastUpdateDate(),
      highestPrice(0.0),
      lowestPrice(0.0),
      previousClosePrice(0.0),
      openPrice(0.0),
      marketInfluence(0.5),
      sectorInfluence(0.3),
      dayChangeAmount(0.0),
      dayChangePercent(0.0)
{
}

Stock::Stock(std::weak_ptr<Company> company, double initialPrice)
    : company(company),
      currentPrice(initialPrice),
      lastUpdateTime(std::time(nullptr)),
      lastUpdateDate(),
      highestPrice(initialPrice),
      lowestPrice(initialPrice),
      previousClosePrice(initialPrice),
      openPrice(initialPrice),
      marketInfluence(0.5),
      sectorInfluence(0.3),
      dayChangeAmount(0.0),
      dayChangePercent(0.0)
{
    priceHistory.push_back(initialPrice);
}

Stock::Stock(const Stock& other)
    : company(other.company),
      currentPrice(other.currentPrice),
      priceHistory(other.priceHistory),
      lastUpdateTime(other.lastUpdateTime),
      lastUpdateDate(other.lastUpdateDate),
      highestPrice(other.highestPrice),
      lowestPrice(other.lowestPrice),
      previousClosePrice(other.previousClosePrice),
      openPrice(other.openPrice),
      marketInfluence(other.marketInfluence),
      sectorInfluence(other.sectorInfluence),
      dayChangeAmount(other.dayChangeAmount),
      dayChangePercent(other.dayChangePercent)
{
}

Stock& Stock::operator=(const Stock& other) {
    if (this != &other) {
        company = other.company;
        currentPrice = other.currentPrice;
        priceHistory = other.priceHistory;
        lastUpdateTime = other.lastUpdateTime;
        lastUpdateDate = other.lastUpdateDate;
        highestPrice = other.highestPrice;
        lowestPrice = other.lowestPrice;
        previousClosePrice = other.previousClosePrice;
        openPrice = other.openPrice;
        marketInfluence = other.marketInfluence;
        sectorInfluence = other.sectorInfluence;
        dayChangeAmount = other.dayChangeAmount;
        dayChangePercent = other.dayChangePercent;
    }
    return *this;
}

double Stock::getCurrentPrice() const {
    return currentPrice;
}

double Stock::getPreviousClosePrice() const {
    return previousClosePrice;
}

double Stock::getOpenPrice() const {
    return openPrice;
}

double Stock::getDayChangeAmount() const {
    return dayChangeAmount;
}

double Stock::getDayChangePercent() const {
    return dayChangePercent;
}

double Stock::getHighestPrice() const {
    return highestPrice;
}

double Stock::getLowestPrice() const {
    return lowestPrice;
}

std::vector<double> Stock::getPriceHistory() const {
    return priceHistory;
}

size_t Stock::getPriceHistoryLength() const {
    return priceHistory.size();
}

std::weak_ptr<Company> Stock::getCompany() const {
    return company;
}

Date Stock::getLastUpdateDate() const {
    return lastUpdateDate;
}

void Stock::updatePrice(double newPrice) {
    if (newPrice <= 0.0) {
        newPrice = 0.01;
    }

    currentPrice = newPrice;

    if (newPrice > highestPrice) {
        highestPrice = newPrice;
    }
    if (newPrice < lowestPrice || lowestPrice == 0.0) {
        lowestPrice = newPrice;
    }

    priceHistory.push_back(newPrice);

    if (priceHistory.size() > 1000) {
        priceHistory.erase(priceHistory.begin());
    }

    lastUpdateTime = std::time(nullptr);
    lastUpdateDate = Date();
    calculateDailyChange();
}

void Stock::calculateDailyChange() {
    dayChangeAmount = currentPrice - openPrice;
    dayChangePercent = (openPrice > 0) ? (dayChangeAmount / openPrice) * 100.0 : 0.0;
}

void Stock::updatePriceWithMarketInfluence(double marketTrend) {
    double influence = marketTrend * marketInfluence;
    double newPrice = currentPrice * (1.0 + influence);
    updatePrice(newPrice);
}

void Stock::updatePriceWithSectorInfluence(double sectorTrend) {
    double influence = sectorTrend * sectorInfluence;
    double newPrice = currentPrice * (1.0 + influence);
    updatePrice(newPrice);
}

void Stock::updatePriceWithNewsImpact(double newsImpact) {
    double newPrice = currentPrice * (1.0 + newsImpact);
    updatePrice(newPrice);
}

void Stock::updatePriceWithVolatility(double volatility) {
    double randomMove = Random::getNormal(0.0, volatility * 0.01);
    double newPrice = currentPrice * (1.0 + randomMove);
    updatePrice(newPrice);
}

void Stock::closeDay() {
    previousClosePrice = currentPrice;
    lastUpdateDate = Date();
}

void Stock::openDay() {
    openPrice = currentPrice;
    dayChangeAmount = 0.0;
    dayChangePercent = 0.0;
    lastUpdateDate = Date();
}

// New methods with Date parameter
void Stock::closeDay(const Date& currentDate) {
    previousClosePrice = currentPrice;
    lastUpdateDate = currentDate;
}

void Stock::openDay(const Date& currentDate) {
    openPrice = currentPrice;
    dayChangeAmount = 0.0;
    dayChangePercent = 0.0;
    lastUpdateDate = currentDate;
}

void Stock::setMarketInfluence(double influence) {
    marketInfluence = std::max(0.0, std::min(1.0, influence));
}

void Stock::setSectorInfluence(double influence) {
    sectorInfluence = std::max(0.0, std::min(1.0, influence));
}

double Stock::generatePriceMovement(double volatility, double marketTrend, double sectorTrend) {
    double randomComponent = Random::getNormal(0.0, volatility * 0.01);

    double marketComponent = marketTrend * marketInfluence;
    double sectorComponent = sectorTrend * sectorInfluence;

    double individualComponent = randomComponent * (1.0 - marketInfluence - sectorInfluence);

    double totalMovement = marketComponent + sectorComponent + individualComponent;

    double newPrice = currentPrice * (1.0 + totalMovement);

    return newPrice;
}

nlohmann::json Stock::toJson() const {
    nlohmann::json j;
    j["current_price"] = currentPrice;
    j["previous_close_price"] = previousClosePrice;
    j["open_price"] = openPrice;
    j["highest_price"] = highestPrice;
    j["lowest_price"] = lowestPrice;
    j["market_influence"] = marketInfluence;
    j["sector_influence"] = sectorInfluence;
    j["price_history"] = priceHistory;
    j["last_update_date"] = lastUpdateDate.toJson();

    return j;
}

Stock Stock::fromJson(const nlohmann::json& json, std::weak_ptr<Company> company) {
    Stock stock;
    stock.company = company;
    stock.currentPrice = json["current_price"];
    stock.previousClosePrice = json["previous_close_price"];
    stock.openPrice = json["open_price"];
    stock.highestPrice = json["highest_price"];
    stock.lowestPrice = json["lowest_price"];
    stock.marketInfluence = json["market_influence"];
    stock.sectorInfluence = json["sector_influence"];

    if (json.contains("price_history") && json["price_history"].is_array()) {
        stock.priceHistory = json["price_history"].get<std::vector<double>>();
    } else {
        stock.priceHistory.push_back(stock.currentPrice);
    }

    if (json.contains("last_update_date")) {
        stock.lastUpdateDate = Date::fromJson(json["last_update_date"]);
    } else {
        stock.lastUpdateDate = Date();
    }

    stock.calculateDailyChange();

    return stock;
}

}