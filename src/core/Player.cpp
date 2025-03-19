#include "Player.hpp"
#include "Market.hpp"
#include <iostream>
#include <algorithm>

namespace StockMarketSimulator {

Player::Player()
    : name("Player"),
      portfolio(std::make_unique<Portfolio>()),
      marginAccountBalance(0.0),
      marginUsed(0.0),
      marginInterestRate(0.07),
      marginAvailable(0.0),
      marginRequirement(0.5),
      currentDay(0)
{
}

Player::Player(const std::string& name, double initialBalance)
    : name(name),
      portfolio(std::make_unique<Portfolio>(initialBalance)),
      marginAccountBalance(0.0),
      marginUsed(0.0),
      marginInterestRate(0.07),
      marginAvailable(0.0),
      marginRequirement(0.5),
      currentDay(0)
{
}

std::string Player::getName() const {
    return name;
}

Portfolio* Player::getPortfolio() const {
    return portfolio.get();
}

double Player::getMarginAccountBalance() const {
    return marginAccountBalance;
}

double Player::getMarginUsed() const {
    return marginUsed;
}

double Player::getMarginAvailable() const {
    return marginAvailable;
}

double Player::getMarginRequirement() const {
    return marginRequirement;
}

double Player::getMarginInterestRate() const {
    return marginInterestRate;
}

double Player::getTotalAssetValue() const {
    return portfolio->getCashBalance() + portfolio->getTotalStocksValue() + marginAccountBalance;
}

double Player::getTotalLiabilities() const {
    double totalLoanDebt = 0.0;
    for (const auto& loan : loans) {
        if (!loan.getIsPaid()) {
            totalLoanDebt += loan.getTotalDue();
        }
    }
    return totalLoanDebt + marginUsed;
}

double Player::getNetWorth() const {
    double assets = getTotalAssetValue();
    double liabilities = getTotalLiabilities();
    return assets - liabilities;
}

const std::vector<Loan>& Player::getLoans() const {
    return loans;
}

int Player::getCurrentDay() const {
    return currentDay;
}

void Player::setName(const std::string& name) {
    this->name = name;
}

void Player::setMarket(std::weak_ptr<Market> market) {
    this->market = market;
}

void Player::setCurrentDay(int day) {
    if (day >= 0) {
        this->currentDay = day;
    }
}

void Player::updateMarginAvailable() {
    double portfolioValue = portfolio->getTotalStocksValue();
    double maxMargin = portfolioValue * (1.0 - marginRequirement);
    double availableMargin = maxMargin + marginAccountBalance - marginUsed;

    marginAvailable = std::max(0.0, availableMargin);
}

    bool Player::checkMarginCall() {
    double portfolioValue = portfolio->getTotalStocksValue();
    double requiredEquity = portfolioValue * marginRequirement;
    double actualEquity = portfolioValue + marginAccountBalance - marginUsed;

    bool isMarginCall = actualEquity < requiredEquity;

    if (!isMarginCall && portfolio->getPreviousDayValue() > 0) {
        double previousStocksValue = portfolio->getPreviousDayValue() - portfolio->getCashBalance();
        double currentStocksValue = portfolioValue;

        if (currentStocksValue < previousStocksValue * 0.7 && marginUsed > 0) {
            isMarginCall = true;
        }
    }

    return isMarginCall;
}
void Player::accrueMarginInterest() {
    if (marginUsed > 0.0) {
        double dailyInterest = marginUsed * (marginInterestRate / 365.0);
        marginUsed += dailyInterest;
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
        return portfolio->buyStock(company, quantity, price, commission, currentDay);
    }

    if (useMargin) {
        updateMarginAvailable();
        double availableCash = portfolio->getCashBalance();

        double totalAvailable = availableCash + marginAvailable;

        if (totalAvailable >= totalCost) {
            double marginNeeded = totalCost - availableCash;

            marginUsed += marginNeeded;

            portfolio->depositCash(marginNeeded);

            bool result = portfolio->buyStock(company, quantity, price, commission, currentDay);

            if (!result) {
                portfolio->withdrawCash(marginNeeded);
                marginUsed -= marginNeeded;
                return false;
            }

            updateMarginAvailable();
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

    bool result = portfolio->sellStock(company, quantity, price, commission, currentDay);

    if (result && marginUsed > 0.0) {
        double cashAfter = portfolio->getCashBalance();
        double sellProceeds = cashAfter - cashBefore;

        double marginToRepay = std::min(marginUsed, sellProceeds);
        if (marginToRepay > 0) {
            marginUsed -= marginToRepay;
            portfolio->withdrawCash(marginToRepay);
        }

        updateMarginAvailable();
    }

    return result;
}

bool Player::takeLoan(double amount, double interestRate, int durationDays, const std::string& description) {
    if (amount <= 0.0 || interestRate < 0.0 || durationDays <= 0) {
        return false;
    }

    double creditLimit = (portfolio->getTotalValue() + marginAccountBalance) * 0.7;
    double currentDebt = getTotalLiabilities();

    if (amount + currentDebt > creditLimit) {
        return false;
    }

    Loan newLoan(amount, interestRate, durationDays, currentDay, 0.001, description);
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

bool Player::depositToMarginAccount(double amount) {
    if (amount <= 0.0) {
        return false;
    }

    if (amount > portfolio->getCashBalance()) {
        return false;
    }

    bool result = portfolio->withdrawCash(amount);
    if (result) {
        marginAccountBalance += amount;
        updateMarginAvailable();
    }

    return result;
}

bool Player::withdrawFromMarginAccount(double amount) {
    if (amount <= 0.0 || amount > marginAccountBalance) {
        return false;
    }
    
    marginAccountBalance -= amount;
    portfolio->depositCash(amount);
    updateMarginAvailable();
    
    return true;
}

bool Player::adjustMarginRequirement(double newRequirement) {
    if (newRequirement < 0.1 || newRequirement > 1.0) {
        return false;
    }
    
    marginRequirement = newRequirement;
    updateMarginAvailable();
    
    return true;
}

bool Player::adjustMarginInterestRate(double newRate) {
    if (newRate < 0.01 || newRate > 0.2) {
        return false;
    }
    
    marginInterestRate = newRate;
    return true;
}

void Player::receiveDividends(std::shared_ptr<Company> company, double amountPerShare) {
    if (!company || amountPerShare <= 0.0) {
        return;
    }
    
    portfolio->receiveDividends(company, amountPerShare);
}

void Player::updateDailyState() {
    portfolio->updatePositionValues();

    processLoans();

    accrueMarginInterest();

    if (checkMarginCall()) {
        double portfolioValue = portfolio->getTotalStocksValue();
        double requiredEquity = portfolioValue * marginRequirement;
        double actualEquity = portfolioValue + marginAccountBalance - marginUsed;
        double deficitAmount = requiredEquity - actualEquity;

        bool shouldSellShares = true;
        double availableCash = portfolio->getCashBalance();

        if (availableCash >= deficitAmount && !shouldSellShares) {
            portfolio->withdrawCash(deficitAmount);
            marginAccountBalance += deficitAmount;
        } else {
            auto marketPtr = market.lock();
            if (!marketPtr) return;

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
                    int sharesToSell = std::max(1, sharesOwned / 10);
                    bool result = sellStock(company, sharesToSell);
                    if (result) {
                        break;
                    }
                }
            }

            double newAvailableCash = portfolio->getCashBalance();
            if (newAvailableCash > 0) {
                double amountToDeposit = std::min(newAvailableCash, deficitAmount);
                if (amountToDeposit > 0) {
                    portfolio->withdrawCash(amountToDeposit);
                    marginAccountBalance += amountToDeposit;
                }
            }
        }
    }

    updateMarginAvailable();
}
void Player::processLoans() {
    for (auto& loan : loans) {
        if (!loan.getIsPaid()) {
            loan.update(currentDay);
        }
    }
}

void Player::closeDay() {
    portfolio->closeDay(currentDay);
    currentDay++;
}

void Player::openDay() {
    portfolio->openDay();
}

nlohmann::json Player::toJson() const {
    nlohmann::json j;
    
    j["name"] = name;
    j["portfolio"] = portfolio->toJson();
    j["margin_account_balance"] = marginAccountBalance;
    j["margin_used"] = marginUsed;
    j["margin_interest_rate"] = marginInterestRate;
    j["margin_requirement"] = marginRequirement;
    j["current_day"] = currentDay;
    
    j["loans"] = nlohmann::json::array();
    for (const auto& loan : loans) {
        j["loans"].push_back(loan.toJson());
    }
    
    return j;
}

Player Player::fromJson(const nlohmann::json& json, std::weak_ptr<Market> market) {
    Player player;
    player.name = json["name"];
    player.marginAccountBalance = json["margin_account_balance"];
    player.marginUsed = json["margin_used"];
    player.marginInterestRate = json["margin_interest_rate"];
    player.marginRequirement = json["margin_requirement"];
    player.currentDay = json["current_day"];
    player.market = market;
    
    auto marketPtr = market.lock();
    if (marketPtr) {
        player.portfolio.reset(new Portfolio(Portfolio::fromJson(json["portfolio"], marketPtr->getCompanies())));
    }
    
    player.loans.clear();
    for (const auto& loanJson : json["loans"]) {
        player.loans.push_back(Loan::fromJson(loanJson));
    }
    
    player.updateMarginAvailable();
    
    return player;
}

}