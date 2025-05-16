#include "Company.hpp"
#include "../utils/Random.hpp"
#include "utils/FileIO.hpp"
#include <stdexcept>

namespace StockMarketSimulator {

DividendPolicy::DividendPolicy()
    : annualDividendRate(0.0),
      paymentFrequency(0),
      daysBetweenPayments(0),
      nextPaymentDay()
{
}

DividendPolicy::DividendPolicy(double rate, int frequency)
    : annualDividendRate(rate),
      paymentFrequency(frequency),
      nextPaymentDay()
{
    if (frequency <= 0) {
        daysBetweenPayments = 0;
    } else {
        daysBetweenPayments = 365 / frequency;
    }
}

bool DividendPolicy::shouldPayDividend(const Date& currentDate) const {
    if (paymentFrequency <= 0 || annualDividendRate <= 0.0) {
        return false;
    }

    if (nextPaymentDay == Date()) {
        return false;
    }

    return currentDate >= nextPaymentDay;
}

bool DividendPolicy::shouldPayDividend(int currentDay) const {
    Date currentDate = Date::fromDayNumber(currentDay);
    return shouldPayDividend(currentDate);
}

double DividendPolicy::calculateDividendAmount() const {
    if (paymentFrequency <= 0) {
        return 0.0;
    }

    return annualDividendRate / paymentFrequency;
}

void DividendPolicy::scheduleNextPayment(const Date& currentDate) {
    if (paymentFrequency <= 0) {
        nextPaymentDay = Date();
    } else {
        nextPaymentDay = currentDate;
        nextPaymentDay.advanceDays(daysBetweenPayments);
    }
}

void DividendPolicy::scheduleNextPayment(int currentDay) {
    Date currentDate = Date::fromDayNumber(currentDay);
    scheduleNextPayment(currentDate);
}

nlohmann::json DividendPolicy::toJson() const {
    nlohmann::json j;
    j["annual_dividend_rate"] = annualDividendRate;
    j["payment_frequency"] = paymentFrequency;
    j["days_between_payments"] = daysBetweenPayments;
    j["next_payment_day"] = nextPaymentDay.toJson();

    return j;
}

DividendPolicy DividendPolicy::fromJson(const nlohmann::json& json) {
    DividendPolicy policy;
    policy.annualDividendRate = json["annual_dividend_rate"];
    policy.paymentFrequency = json["payment_frequency"];
    policy.daysBetweenPayments = json["days_between_payments"];

    if (json.contains("next_payment_day")) {
        policy.nextPaymentDay = Date::fromJson(json["next_payment_day"]);
    } else if (json.contains("next_payment_day_int")) {
        int nextPaymentDayInt = json["next_payment_day_int"];
        policy.nextPaymentDay = Date::fromDayNumber(nextPaymentDayInt);
    } else {
        policy.nextPaymentDay = Date();
    }

    return policy;
}

Sector Company::sectorFromString(const std::string& sectorStr) {
    if (sectorStr == "Technology") return Sector::Technology;
    if (sectorStr == "Energy") return Sector::Energy;
    if (sectorStr == "Finance") return Sector::Finance;
    if (sectorStr == "Consumer") return Sector::Consumer;
    if (sectorStr == "Manufacturing") return Sector::Manufacturing;

    return Sector::Unknown;
}

std::string Company::sectorToString(Sector sector) {
    switch (sector) {
        case Sector::Technology: return "Technology";
        case Sector::Energy: return "Energy";
        case Sector::Finance: return "Finance";
        case Sector::Consumer: return "Consumer";
        case Sector::Manufacturing: return "Manufacturing";
        default: return "Unknown";
    }
}

Company::Company()
    : name(""),
      ticker(""),
      description(""),
      sector(Sector::Unknown),
      volatility(0.0),
      marketCap(0.0),
      peRatio(0.0),
      revenue(0.0),
      profit(0.0)
{
    stock = std::make_unique<Stock>(weak_from_this(), 0.0);
}

Company::Company(const std::string& name, const std::string& ticker,
                 const std::string& description, Sector sector,
                 double initialPrice, double volatility,
                 const DividendPolicy& dividendPolicy)
    : name(name),
      ticker(ticker),
      description(description),
      sector(sector),
      volatility(volatility),
      dividendPolicy(dividendPolicy),
      marketCap(0.0),
      peRatio(0.0),
      revenue(0.0),
      profit(0.0),
      lastDividendDate()
{
    stock = std::make_unique<Stock>(weak_from_this(), initialPrice);

    marketCap = initialPrice * 1000000;
}

Company::Company(const Company& other)
    : name(other.name),
      ticker(other.ticker),
      description(other.description),
      sector(other.sector),
      volatility(other.volatility),
      dividendPolicy(other.dividendPolicy),
      marketCap(other.marketCap),
      peRatio(other.peRatio),
      revenue(other.revenue),
      profit(other.profit)
{
    if (other.stock) {
        stock = std::make_unique<Stock>(*other.stock);
        stock->getCompany() = weak_from_this();
    } else {
        stock = std::make_unique<Stock>(weak_from_this(), 0.0);
    }
}

Company& Company::operator=(const Company& other) {
    if (this != &other) {
        name = other.name;
        ticker = other.ticker;
        description = other.description;
        sector = other.sector;
        volatility = other.volatility;
        dividendPolicy = other.dividendPolicy;
        marketCap = other.marketCap;
        peRatio = other.peRatio;
        revenue = other.revenue;
        profit = other.profit;

        if (other.stock) {
            stock = std::make_unique<Stock>(*other.stock);
            stock->getCompany() = weak_from_this();
        } else {
            stock = std::make_unique<Stock>(weak_from_this(), 0.0);
        }
    }
    return *this;
}

std::string Company::getName() const {
    return name;
}

std::string Company::getTicker() const {
    return ticker;
}

std::string Company::getDescription() const {
    return description;
}

Sector Company::getSector() const {
    return sector;
}

std::string Company::getSectorName() const {
    return sectorToString(sector);
}

double Company::getVolatility() const {
    return volatility;
}

const DividendPolicy& Company::getDividendPolicy() const {
    return dividendPolicy;
}

Stock* Company::getStock() const {
    return stock.get();
}

double Company::getMarketCap() const {
    return marketCap;
}

double Company::getPERatio() const {
    return peRatio;
}

double Company::getRevenue() const {
    return revenue;
}

double Company::getProfit() const {
    return profit;
}

void Company::setName(const std::string& name) {
    this->name = name;
}

void Company::setTicker(const std::string& ticker) {
    this->ticker = ticker;
}

void Company::setDescription(const std::string& description) {
    this->description = description;
}

void Company::setSector(Sector sector) {
    this->sector = sector;
}

void Company::setVolatility(double volatility) {
    this->volatility = std::max(0.0, std::min(1.0, volatility));
}

void Company::setDividendPolicy(const DividendPolicy& policy) {
    this->dividendPolicy = policy;
}

void Company::setFinancials(double marketCap, double peRatio, double revenue, double profit) {
    this->marketCap = marketCap;
    this->peRatio = peRatio;
    this->revenue = revenue;
    this->profit = profit;
}

void Company::updateStockPrice(double marketTrend, double sectorTrend) {
    if (!stock) {
        return;
    }

    double newPrice = stock->generatePriceMovement(volatility, marketTrend, sectorTrend);
    stock->updatePrice(newPrice);

    marketCap = stock->getCurrentPrice() * 1000000;
}

void Company::processNewsImpact(double newsImpact) {
    if (!stock) {
        return;
    }

    stock->updatePriceWithNewsImpact(newsImpact);

    marketCap = stock->getCurrentPrice() * 1000000;
}

void Company::closeTradingDay(const Date& currentDate) {
    if (stock) {
        stock->closeDay(currentDate);
    }
}

void Company::closeTradingDay() {
    if (stock) {
        stock->closeDay();
    }
}

void Company::openTradingDay(const Date& currentDate) {
    if (stock) {
        stock->openDay(currentDate);
    }
}

void Company::openTradingDay() {
    if (stock) {
        stock->openDay();
    }
}

bool Company::processDividends(const Date& currentDate) {
    if (dividendPolicy.shouldPayDividend(currentDate)) {
        lastDividendDate = currentDate;

        dividendPolicy.scheduleNextPayment(currentDate);
        return true;
    }
    return false;
}

bool Company::processDividends(int currentDay) {
    Date currentDate = Date::fromDayNumber(currentDay);
    return processDividends(currentDate);
}

double Company::calculateDividendAmount() const {
    return dividendPolicy.calculateDividendAmount();
}

nlohmann::json Company::toJson() const {
    nlohmann::json j;
    j["name"] = name;
    j["ticker"] = ticker;
    j["description"] = description;
    j["sector"] = sectorToString(sector);
    j["volatility"] = volatility;
    j["dividend_policy"] = dividendPolicy.toJson();
    j["market_cap"] = marketCap;
    j["pe_ratio"] = peRatio;
    j["revenue"] = revenue;
    j["profit"] = profit;

    if (stock) {
        j["stock"] = stock->toJson();
    }

    return j;
}

std::shared_ptr<Company> Company::fromJson(const nlohmann::json& json) {
    auto company = std::make_shared<Company>();

    company->name = json["name"];
    company->ticker = json["ticker"];
    company->description = json["description"];
    company->sector = sectorFromString(json["sector"]);
    company->volatility = json["volatility"];
    company->dividendPolicy = DividendPolicy::fromJson(json["dividend_policy"]);
    company->marketCap = json["market_cap"];
    company->peRatio = json["pe_ratio"];
    company->revenue = json["revenue"];
    company->profit = json["profit"];

    if (json.contains("stock")) {
        company->stock.reset(new Stock(Stock::fromJson(json["stock"], company)));
    } else {
        company->stock = std::make_unique<Stock>(company, 0.0);
    }

    return company;
}

void Company::initializeDividendSchedule(const Date& currentDate) {
    if (dividendPolicy.paymentFrequency > 0 && dividendPolicy.nextPaymentDay == Date()) {
        std::stringstream logMsg;
        logMsg << "Initializing dividend schedule for " << name
               << " - Annual rate: " << dividendPolicy.annualDividendRate
               << "$ - Frequency: " << dividendPolicy.paymentFrequency
               << " times per year";
        FileIO::appendToLog(logMsg.str());

        dividendPolicy.scheduleNextPayment(currentDate);

        std::stringstream logMsg2;
        logMsg2 << "Dividend schedule for " << name << " initialized. Next payment on: "
                << dividendPolicy.nextPaymentDay.toString();
        FileIO::appendToLog(logMsg2.str());
    }
}

}
