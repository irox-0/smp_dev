#include "Player.hpp"
#include "Market.hpp"
#include "utils/FileIO.hpp"
#include <iostream>
#include <algorithm>

namespace StockMarketSimulator {

Player::Player()
    : name("Player"),
      portfolio(std::make_unique<Portfolio>()),
      marginLoan(0.0),
      marginInterestRate(0.07),
      marginLimitMultiplier(2.0),
      currentDate()
{
}

Player::Player(const std::string& name, double initialBalance)
    : name(name),
      portfolio(std::make_unique<Portfolio>(initialBalance)),
      marginLoan(0.0),
      marginInterestRate(0.07),
      marginLimitMultiplier(2.0),
      currentDate()
{
}

std::string Player::getName() const {
    return name;
}

Portfolio* Player::getPortfolio() const {
    return portfolio.get();
}

double Player::getMarginLoan() const {
    return marginLoan;
}

double Player::getMarginInterestRate() const {
    return marginInterestRate;
}

double Player::getMaxMarginLoan() const {
    double portfolioValue = portfolio->getTotalValue();
    return portfolioValue * marginLimitMultiplier - marginLoan;
}

double Player::getTotalAssetValue() const {
    return portfolio->getCashBalance() + portfolio->getTotalStocksValue();
}

double Player::getTotalLiabilities() const {
    double totalLoanDebt = 0.0;
    for (const auto& loan : loans) {
        if (!loan.getIsPaid()) {
            totalLoanDebt += loan.getTotalDue();
        }
    }
    return totalLoanDebt + marginLoan;
}

double Player::getNetWorth() const {
    double assets = getTotalAssetValue();
    double liabilities = getTotalLiabilities();
    return assets - liabilities;
}

const std::vector<Loan>& Player::getLoans() const {
    return loans;
}

Date Player::getCurrentDate() const {
    return currentDate;
}

int Player::getCurrentDay() const {
    Date referenceDate(1, 3, 2023);
    return currentDate.toDayNumber(referenceDate);
}

void Player::setName(const std::string& name) {
    this->name = name;
}

void Player::setMarket(std::weak_ptr<Market> market) {
    this->market = market;
}

void Player::setCurrentDate(const Date& date) {
    this->currentDate = date;
}

bool Player::checkMarginCall() const {
    if (marginLoan <= 0.0) {
        return false;
    }

    double totalAssets = portfolio->getTotalValue();
    return totalAssets < marginLoan;
}

void Player::accrueMarginInterest() {
    if (marginLoan > 0.0) {
        double dailyInterest = marginLoan * (marginInterestRate / 7.0);
        marginLoan += dailyInterest;
    }
}

bool Player::buyStock(std::shared_ptr<Company> company, int quantity, bool useMargin) {
    if (!company || quantity <= 0) {
        return false;
    }

    auto marketPtr = market.lock();
    if (!marketPtr) {
        return false;
    }

    double price = company->getStock()->getCurrentPrice();
    double commission = 0.01;
    double totalCost = price * quantity * (1.0 + commission);

    if (totalCost <= portfolio->getCashBalance()) {
        return portfolio->buyStock(company, quantity, price, commission, currentDate);
    }

    if (useMargin) {
        double availableCash = portfolio->getCashBalance();
        double marginNeeded = totalCost - availableCash;

        if (marginNeeded <= getMaxMarginLoan()) {
            marginLoan += marginNeeded;

            portfolio->depositCash(marginNeeded);

            bool result = portfolio->buyStock(company, quantity, price, commission, currentDate);

            if (!result) {
                marginLoan -= marginNeeded;
                portfolio->withdrawCash(marginNeeded);
                return false;
            }

            return true;
        }
    }

    return false;
}

bool Player::sellStock(std::shared_ptr<Company> company, int quantity) {
    if (!company || quantity <= 0) {
        return false;
    }

    std::string ticker = company->getTicker();
    if (!portfolio->hasPosition(ticker) || portfolio->getPositionQuantity(ticker) < quantity) {
        return false;
    }

    double price = company->getStock()->getCurrentPrice();
    double commission = 0.01;

    double cashBefore = portfolio->getCashBalance();

    bool result = portfolio->sellStock(company, quantity, price, commission, currentDate);

    if (result && marginLoan > 0.0) {
        double cashAfter = portfolio->getCashBalance();
        double sellProceeds = cashAfter - cashBefore;

    }

    return result;
}

bool Player::takeLoan(double amount, double interestRate, int durationDays, const std::string& description) {
    if (amount <= 0.0 || interestRate < 0.0 || durationDays <= 0) {
        return false;
    }

    double creditLimit = (portfolio->getTotalValue()) * 0.7;
    double currentDebt = getTotalLiabilities();

    if (amount + currentDebt > creditLimit) {
        return false;
    }

    Loan newLoan(amount, interestRate, durationDays, currentDate, 0.001, description);
    loans.push_back(newLoan);

    portfolio->depositCash(amount);

    return true;
}

bool Player::repayLoan(size_t loanIndex, double amount) {
    if (loanIndex >= loans.size() || amount <= 0.0) {
        return false;
    }

    Loan& loan = loans[loanIndex];

    if (loan.getIsPaid()) {
        return false;
    }

    double totalDue = loan.getTotalDue();
    double amountToRepay = std::min(amount, totalDue);

    if (amountToRepay > portfolio->getCashBalance()) {
        return false;
    }

    portfolio->withdrawCash(amountToRepay);

    if (amountToRepay >= totalDue) {
        loan.markAsPaid();
    }

    return true;
}

bool Player::depositCash(double amount) {
    if (amount <= 0.0) {
        return false;
    }

    portfolio->depositCash(amount);
    return true;
}

bool Player::withdrawCash(double amount) {
    if (amount <= 0.0) {
        return false;
    }

    return portfolio->withdrawCash(amount);
}

bool Player::takeMarginLoan(double amount) {
    if (amount <= 0.0) {
        return false;
    }

    double maxLoan = getMaxMarginLoan();
    if (amount > maxLoan) {
        return false;
    }

    marginLoan += amount;
    portfolio->depositCash(amount);
    return true;
}

bool Player::repayMarginLoan(double amount) {
    if (amount <= 0.0 || amount > marginLoan) {
        return false;
    }

    if (amount > portfolio->getCashBalance()) {
        return false;
    }

    marginLoan -= amount;
    portfolio->withdrawCash(amount);
    return true;
}

// void Player::receiveDividends(std::shared_ptr<Company> company, double amountPerShare) {
//     if (!company || amountPerShare <= 0.0) {
//         return;
//     }
//
//     std::string ticker = company->getTicker();
//     if (portfolio->hasPosition(ticker)) {
//         int shares = portfolio->getPositionQuantity(ticker);
//         double totalAmount = shares * amountPerShare;
//
//         std::stringstream logMsg;
//         logMsg << "Player received dividend: " << totalAmount
//                << "$ for " << shares << " shares of "
//                << company->getName();
//         FileIO::appendToLog(logMsg.str());
//     }
//
//     portfolio->receiveDividends(company, amountPerShare);
// }

void Player::updateDailyState() {
    portfolio->updatePositionValues();
    portfolio->checkDividendPayments(currentDate);
    processLoans();

    accrueMarginInterest();

    if (checkMarginCall()) {
        auto marketPtr = market.lock();
        if (marketPtr) {
            std::vector<std::pair<std::string, double>> positions;




            for (const auto& [ticker, position] : portfolio->getPositions()) {
                positions.push_back({ticker, position.unrealizedProfitLossPercent});
            }

            std::sort(positions.begin(), positions.end(),
                    [](const auto& a, const auto& b) { return a.second < b.second; });

            for (const auto& [ticker, _] : positions) {
                auto company = marketPtr->getCompanyByTicker(ticker);
                if (!company) continue;

                int sharesOwned = portfolio->getPositionQuantity(ticker);
                if (sharesOwned > 0) {
                    int sharesToSell = std::max(1, sharesOwned / 2);
                    sellStock(company, sharesToSell);

                    if (!checkMarginCall()) {
                        break;
                    }
                }
            }

            if (checkMarginCall() && portfolio->getCashBalance() > 0) {
                double amountToRepay = std::min(portfolio->getCashBalance(), marginLoan);
                repayMarginLoan(amountToRepay);
            }
        }
    }
}

void Player::processLoans() {
    for (auto& loan : loans) {
        if (!loan.getIsPaid()) {
            loan.update(currentDate);

            if (loan.getDueDate() == currentDate) {
                double totalDue = loan.getTotalDue();

                if (portfolio->getCashBalance() >= totalDue) {
                    portfolio->withdrawCash(totalDue);
                    loan.markAsPaid();
                }
            }
        }
    }
}

void Player::closeDay() {
    portfolio->closeDay(currentDate);
    currentDate.nextDay();
}

void Player::openDay() {
    portfolio->openDay();
}

nlohmann::json Player::toJson() const {
    nlohmann::json j;

    j["name"] = name;
    j["portfolio"] = portfolio->toJson();
    j["margin_loan"] = marginLoan;
    j["margin_interest_rate"] = marginInterestRate;
    j["margin_limit_multiplier"] = marginLimitMultiplier;
    j["current_date"] = currentDate.toJson();

    j["loans"] = nlohmann::json::array();
    for (const auto& loan : loans) {
        j["loans"].push_back(loan.toJson());
    }

    return j;
}

Player Player::fromJson(const nlohmann::json& json, std::weak_ptr<Market> market) {
    Player player;
    player.name = json["name"];

    if (json.contains("margin_loan")) {
        player.marginLoan = json["margin_loan"];
    } else if (json.contains("margin_used")) {
        player.marginLoan = json["margin_used"];
    } else {
        player.marginLoan = 0.0;
    }

    if (json.contains("margin_interest_rate")) {
        player.marginInterestRate = json["margin_interest_rate"];
    } else {
        player.marginInterestRate = 0.07;
    }

    if (json.contains("margin_limit_multiplier")) {
        player.marginLimitMultiplier = json["margin_limit_multiplier"];
    } else {
        player.marginLimitMultiplier = 2.0;
    }

    if (json.contains("current_date")) {
        player.currentDate = Date::fromJson(json["current_date"]);
    } else if (json.contains("current_day")) {
        int currentDay = json["current_day"];
        player.currentDate = Date::fromDayNumber(currentDay);
    } else {
        player.currentDate = Date();
    }

    player.market = market;

    auto marketPtr = market.lock();
    if (marketPtr) {
        player.portfolio.reset(new Portfolio(Portfolio::fromJson(json["portfolio"], marketPtr->getCompanies())));
    }

    player.loans.clear();
    for (const auto& loanJson : json["loans"]) {
        player.loans.push_back(Loan::fromJson(loanJson));
    }
    
    return player;
}

}