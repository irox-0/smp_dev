#include "Portfolio.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace StockMarketSimulator {


PortfolioPosition::PortfolioPosition()
    : quantity(0),
      averagePurchasePrice(0.0),
      totalCost(0.0),
      currentValue(0.0),
      unrealizedProfitLoss(0.0),
      unrealizedProfitLossPercent(0.0)
{
}

PortfolioPosition::PortfolioPosition(std::shared_ptr<Company> company, int quantity, double price)
    : company(company),
      quantity(quantity),
      averagePurchasePrice(price),
      totalCost(quantity * price),
      currentValue(quantity * company->getStock()->getCurrentPrice()),
      unrealizedProfitLoss(0.0),
      unrealizedProfitLossPercent(0.0)
{
    calculateUnrealizedProfitLoss();
}

void PortfolioPosition::updatePosition(int addedQuantity, double price) {
    if (quantity + addedQuantity <= 0) {
        throw std::runtime_error("Cannot update position to zero or negative quantity");
    }

    double newTotalCost = totalCost + (addedQuantity * price);
    quantity += addedQuantity;
    averagePurchasePrice = newTotalCost / quantity;
    totalCost = newTotalCost;

    updateCurrentValue();
}

void PortfolioPosition::updateCurrentValue() {
    if (!company || !company->getStock()) {
        currentValue = 0.0;
        unrealizedProfitLoss = -totalCost;
        unrealizedProfitLossPercent = -100.0;
        return;
    }

    currentValue = quantity * company->getStock()->getCurrentPrice();
    calculateUnrealizedProfitLoss();
}

void PortfolioPosition::calculateUnrealizedProfitLoss() {
    unrealizedProfitLoss = currentValue - totalCost;

    if (totalCost > 0) {
        unrealizedProfitLossPercent = (unrealizedProfitLoss / totalCost) * 100.0;
    } else {
        unrealizedProfitLossPercent = 0.0;
    }
}

nlohmann::json PortfolioPosition::toJson() const {
    nlohmann::json j;

    j["ticker"] = company ? company->getTicker() : "";
    j["quantity"] = quantity;
    j["average_purchase_price"] = averagePurchasePrice;
    j["total_cost"] = totalCost;
    j["current_value"] = currentValue;
    j["unrealized_profit_loss"] = unrealizedProfitLoss;
    j["unrealized_profit_loss_percent"] = unrealizedProfitLossPercent;

    return j;
}

PortfolioPosition PortfolioPosition::fromJson(const nlohmann::json& json, std::shared_ptr<Company> company) {
    PortfolioPosition position;

    position.company = company;
    position.quantity = json["quantity"];
    position.averagePurchasePrice = json["average_purchase_price"];
    position.totalCost = json["total_cost"];
    position.currentValue = json["current_value"];
    position.unrealizedProfitLoss = json["unrealized_profit_loss"];
    position.unrealizedProfitLossPercent = json["unrealized_profit_loss_percent"];

    return position;
}


PortfolioHistory::PortfolioHistory()
    : day(0),
      totalValue(0.0),
      cashBalance(0.0),
      totalReturn(0.0),
      totalReturnPercent(0.0)
{
}

PortfolioHistory::PortfolioHistory(int day, double totalValue, double cashBalance, double totalReturn, double totalReturnPercent)
    : day(day),
      totalValue(totalValue),
      cashBalance(cashBalance),
      totalReturn(totalReturn),
      totalReturnPercent(totalReturnPercent)
{
}

nlohmann::json PortfolioHistory::toJson() const {
    nlohmann::json j;

    j["day"] = day;
    j["total_value"] = totalValue;
    j["cash_balance"] = cashBalance;
    j["total_return"] = totalReturn;
    j["total_return_percent"] = totalReturnPercent;

    return j;
}

PortfolioHistory PortfolioHistory::fromJson(const nlohmann::json& json) {
    PortfolioHistory history;

    history.day = json["day"];
    history.totalValue = json["total_value"];
    history.cashBalance = json["cash_balance"];
    history.totalReturn = json["total_return"];
    history.totalReturnPercent = json["total_return_percent"];

    return history;
}


Portfolio::Portfolio()
    : initialInvestment(0.0),
      cashBalance(0.0),
      totalValue(0.0),
      previousDayValue(0.0),
      totalDividendsReceived(0.0)
{
}

Portfolio::Portfolio(double initialBalance)
    : initialInvestment(initialBalance),
      cashBalance(initialBalance),
      totalValue(initialBalance),
      previousDayValue(initialBalance),
      totalDividendsReceived(0.0)
{
}

double Portfolio::getInitialInvestment() const {
    return initialInvestment;
}

double Portfolio::getCashBalance() const {
    return cashBalance;
}

double Portfolio::getTotalValue() const {
    return totalValue;
}

double Portfolio::getTotalStocksValue() const {
    double stocksValue = 0.0;
    for (const auto& [ticker, position] : positions) {
        stocksValue += position.currentValue;
    }
    return stocksValue;
}

double Portfolio::getPreviousDayValue() const {
    return previousDayValue;
}

double Portfolio::getTotalDividendsReceived() const {
    return totalDividendsReceived;
}

double Portfolio::getTotalReturn() const {
    return totalValue - initialInvestment;
}

double Portfolio::getTotalReturnPercent() const {
    if (initialInvestment > 0.0) {
        return (totalValue - initialInvestment) / initialInvestment * 100.0;
    }
    return 0.0;
}

double Portfolio::getDayChangeAmount() const {
    return totalValue - previousDayValue;
}

double Portfolio::getDayChangePercent() const {
    if (previousDayValue > 0) {
        return (getDayChangeAmount() / previousDayValue) * 100.0;
    }
    return 0.0;
}

const std::unordered_map<std::string, PortfolioPosition>& Portfolio::getPositions() const {
    return positions;
}

const std::vector<PortfolioHistory>& Portfolio::getHistory() const {
    return history;
}

const std::vector<Transaction>& Portfolio::getTransactions() const {
    return transactions;
}

bool Portfolio::hasPosition(const std::string& ticker) const {
    return positions.find(ticker) != positions.end();
}

int Portfolio::getPositionQuantity(const std::string& ticker) const {
    auto it = positions.find(ticker);
    if (it != positions.end()) {
        return it->second.quantity;
    }
    return 0;
}

const PortfolioPosition* Portfolio::getPosition(const std::string& ticker) const {
    auto it = positions.find(ticker);
    if (it != positions.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool Portfolio::buyStock(std::shared_ptr<Company> company, int quantity, double price, double commission, int day) {
    if (!company || quantity <= 0 || price <= 0) {
        return false;
    }

    double baseCost = quantity * price;
    double commissionAmount = baseCost * commission;
    double totalCost = baseCost + commissionAmount;

    if (totalCost > cashBalance) {
        return false;
    }

    Transaction transaction(TransactionType::Buy, company, quantity, price, commission, day);

    cashBalance -= totalCost;

    std::string ticker = company->getTicker();
    if (hasPosition(ticker)) {
        positions[ticker].updatePosition(quantity, price);
    } else {
        positions[ticker] = PortfolioPosition(company, quantity, price);
    }

    updatePortfolioValue();

    transaction.execute();
    transactions.push_back(transaction);

    return true;
}
bool Portfolio::sellStock(std::shared_ptr<Company> company, int quantity, double price, double commission, int day) {
    if (!company || quantity <= 0 || price <= 0) {
        return false;
    }

    std::string ticker = company->getTicker();
    if (!hasPosition(ticker) || getPositionQuantity(ticker) < quantity) {
        return false;
    }

    Transaction transaction(TransactionType::Sell, company, quantity, price, commission, day);
    if (!transaction.validateSellTransaction(getPositionQuantity(ticker))) {
        return false;
    }

    PortfolioPosition& position = positions[ticker];
    double sellValue = price * quantity;
    double commissionAmount = sellValue * commission;
    double netSellValue = sellValue - commissionAmount;

    cashBalance += netSellValue;

    if (position.quantity == quantity) {
        positions.erase(ticker);
    } else {
        // Уменьшаем позицию
        double remainingCost = position.totalCost * (static_cast<double>(position.quantity - quantity) / position.quantity);
        position.quantity -= quantity;
        position.totalCost = remainingCost;
        position.averagePurchasePrice = remainingCost / position.quantity;
        position.updateCurrentValue();
    }

    updatePortfolioValue();

    transaction.execute();
    transactions.push_back(transaction);

    return true;
}

void Portfolio::updatePositionValues() {
    for (auto& [ticker, position] : positions) {
        position.updateCurrentValue();
    }

    updatePortfolioValue();
}

void Portfolio::updatePortfolioValue() {
    double stocksValue = 0.0;
    for (auto& [ticker, position] : positions) {
        position.updateCurrentValue();
        stocksValue += position.currentValue;
    }
    totalValue = cashBalance + stocksValue;
}

void Portfolio::closeDay(int day) {
    updatePositionValues();
    recordHistoryEntry(day);
}

void Portfolio::openDay() {
    previousDayValue = totalValue;
}

void Portfolio::recordHistoryEntry(int day) {
    double totalReturn = getTotalReturn();
    double totalReturnPercent = getTotalReturnPercent();

    PortfolioHistory entry(day, totalValue, cashBalance, totalReturn, totalReturnPercent);
    history.push_back(entry);

    const size_t MAX_HISTORY_SIZE = 365 * 5;
    if (history.size() > MAX_HISTORY_SIZE) {
        history.erase(history.begin());
    }
}

void Portfolio::receiveDividends(std::shared_ptr<Company> company, double amount) {
    if (!company || amount <= 0) {
        return;
    }

    std::string ticker = company->getTicker();
    if (!hasPosition(ticker)) {
        return;
    }

    int shares = positions[ticker].quantity;
    double dividendAmount = shares * amount;

    cashBalance += dividendAmount;
    totalDividendsReceived += dividendAmount;
    updatePortfolioValue();
}

void Portfolio::depositCash(double amount) {
    if (amount <= 0) {
        return;
    }

    cashBalance += amount;
    initialInvestment += amount;
    updatePortfolioValue();
}

bool Portfolio::withdrawCash(double amount) {
    if (amount <= 0 || amount > cashBalance) {
        return false;
    }

    cashBalance -= amount;
    updatePortfolioValue();
    return true;
}

std::map<Sector, double> Portfolio::getSectorAllocation() const {
    std::map<Sector, double> allocation;

    for (const auto& [ticker, position] : positions) {
        Sector sector = position.company->getSector();
        allocation[sector] += position.currentValue;
    }

    return allocation;
}

double Portfolio::getSectorAllocationPercent(Sector sector) const {
    double sectorValue = 0.0;
    double stocksValue = getTotalStocksValue();

    if (stocksValue <= 0) {
        return 0.0;
    }

    for (const auto& [ticker, position] : positions) {
        if (position.company->getSector() == sector) {
            sectorValue += position.currentValue;
        }
    }

    return (sectorValue / stocksValue) * 100.0;
}

std::vector<double> Portfolio::getValueHistory() const {
    std::vector<double> values;
    values.reserve(history.size());

    for (const auto& entry : history) {
        values.push_back(entry.totalValue);
    }

    return values;
}

double Portfolio::getPeriodReturn(int days) const {
    if (history.empty() || days <= 0 || static_cast<size_t>(days) >= history.size()) {
        return getTotalReturn();
    }

    size_t historyIndex = history.size() - 1 - static_cast<size_t>(days);
    double pastValue = history[historyIndex].totalValue;

    return totalValue - pastValue;
}

double Portfolio::getPeriodReturnPercent(int days) const {
    if (history.empty() || days <= 0 || static_cast<size_t>(days) >= history.size()) {
        return getTotalReturnPercent();
    }

    size_t historyIndex = history.size() - 1 - static_cast<size_t>(days);
    double pastValue = history[historyIndex].totalValue;

    if (pastValue <= 0.0) {
        return 0.0;
    }

    return ((totalValue - pastValue) / pastValue) * 100.0;
}

nlohmann::json Portfolio::toJson() const {
    nlohmann::json j;

    j["initial_investment"] = initialInvestment;
    j["cash_balance"] = cashBalance;
    j["total_value"] = totalValue;
    j["previous_day_value"] = previousDayValue;
    j["total_dividends_received"] = totalDividendsReceived;

    j["positions"] = nlohmann::json::array();
    for (const auto& [ticker, position] : positions) {
        nlohmann::json positionJson = position.toJson();
        j["positions"].push_back(positionJson);
    }

    j["history"] = nlohmann::json::array();
    for (const auto& entry : history) {
        j["history"].push_back(entry.toJson());
    }

    j["transactions"] = nlohmann::json::array();
    for (const auto& transaction : transactions) {
        j["transactions"].push_back(transaction.toJson());
    }

    return j;
}

Portfolio Portfolio::fromJson(const nlohmann::json& json, const std::vector<std::shared_ptr<Company>>& allCompanies) {
    Portfolio portfolio;

    portfolio.initialInvestment = json["initial_investment"];
    portfolio.cashBalance = json["cash_balance"];
    portfolio.totalValue = json["total_value"];
    portfolio.previousDayValue = json["previous_day_value"];
    portfolio.totalDividendsReceived = json["total_dividends_received"];

    std::unordered_map<std::string, std::shared_ptr<Company>> companyMap;
    for (const auto& company : allCompanies) {
        companyMap[company->getTicker()] = company;
    }

    for (const auto& positionJson : json["positions"]) {
        std::string ticker = positionJson["ticker"];
        if (companyMap.find(ticker) != companyMap.end()) {
            PortfolioPosition position = PortfolioPosition::fromJson(positionJson, companyMap[ticker]);
            portfolio.positions[ticker] = position;
        }
    }

    for (const auto& historyJson : json["history"]) {
        PortfolioHistory historyEntry = PortfolioHistory::fromJson(historyJson);
        portfolio.history.push_back(historyEntry);
    }

    for (const auto& transactionJson : json["transactions"]) {
        Transaction transaction = Transaction::fromJson(transactionJson);
        std::string ticker = transactionJson["company_ticker"];
        if (companyMap.find(ticker) != companyMap.end()) {
            transaction.setCompany(companyMap[ticker]);
        }
        portfolio.transactions.push_back(transaction);
    }

    return portfolio;
}

}