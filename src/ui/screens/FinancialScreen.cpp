#include "FinancialScreen.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace StockMarketSimulator {

const int MAX_LOANS = 4;

FinancialScreen::FinancialScreen()
    : Screen("FINANCIAL INSTRUMENTS", ScreenType::Financial),
      currentSection(FinancialSection::Loans),
      selectedLoanIndex(-1)
{
    setSize(52, 30);
}

void FinancialScreen::initialize() {
    Screen::initialize();

    loansTable.setPosition(x, y + 7);
    std::vector<std::string> loanHeaders = {"Loan #", "Amount", "Interest", "Due In", "Total Due"};
    loansTable.setHeaders(loanHeaders);
    std::vector<int> loanColumnWidths = {7, 10, 9, 10, 10};
    loansTable.setColumnWidths(loanColumnWidths);
    loansTable.setHeaderColors(TextColor::White, TextColor::Blue);
    loansTable.setBodyColors(bodyFg, bodyBg);

    marginTable.setPosition(x + 2, y + 11);
    std::vector<std::string> marginHeaders = {"Used", "Available", "Rate"};
    marginTable.setHeaders(marginHeaders);
    std::vector<int> marginColumnWidths = {12, 12, 8};
    marginTable.setColumnWidths(marginColumnWidths);
    marginTable.setHeaderColors(TextColor::White, TextColor::Blue);
    marginTable.setBodyColors(bodyFg, bodyBg);

    update();
}

void FinancialScreen::update() {
    Screen::update();
    updateLoansTable();
    updateMarginTable();
}

void FinancialScreen::updateLoansTable() {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    loansTable.clearData();

    const std::vector<Loan>& loans = playerPtr->getLoans();
    int currentDay = playerPtr->getCurrentDay();

    for (size_t i = 0; i < loans.size(); i++) {
        const Loan& loan = loans[i];

        if (loan.getIsPaid()) {
            continue;
        }

        std::stringstream amountStr;
        amountStr << std::fixed << std::setprecision(2) << loan.getAmount() << "$";

        std::stringstream interestStr;
        interestStr << std::fixed << std::setprecision(1) << (loan.getInterestRate() * 100.0) << "%";

        int daysRemaining = loan.daysRemaining(currentDay);
        std::string dueInStr = std::to_string(daysRemaining) + " days";

        std::stringstream totalDueStr;
        totalDueStr << std::fixed << std::setprecision(2) << loan.getTotalDue() << "$";

        std::vector<std::string> row = {
            "#" + std::to_string(i + 1),
            amountStr.str(),
            interestStr.str(),
            dueInStr,
            totalDueStr.str()
        };

        loansTable.addRow(row);
    }
}

void FinancialScreen::updateMarginTable() {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    marginTable.clearData();

    std::stringstream usedStr;
    usedStr << std::fixed << std::setprecision(2) << playerPtr->getMarginUsed() << "$";

    std::stringstream availableStr;
    availableStr << std::fixed << std::setprecision(2) << playerPtr->getMarginAvailable() << "$";

    std::stringstream rateStr;
    rateStr << std::fixed << std::setprecision(1) << (playerPtr->getMarginInterestRate() * 100.0) << "%";

    std::vector<std::string> row = {
        usedStr.str(),
        availableStr.str(),
        rateStr.str()
    };

    marginTable.addRow(row);
}

void FinancialScreen::drawContent() const {
    drawFinancialInfo();
    drawSectionTitle();

    if (currentSection == FinancialSection::Loans) {
        drawCurrentObligations();
        drawAvailableLoans();
    } else {
        drawMarginInfo();
    }

    drawNavigationOptions();
}

void FinancialScreen::drawFinancialInfo() const {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    Console::setCursorPosition(x + 2, y + 2);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Available Funds: ");
    Console::setColor(TextColor::Green, bodyBg);

    std::stringstream fundsStr;
    fundsStr << std::fixed << std::setprecision(2) << playerPtr->getPortfolio()->getCashBalance() << "$";
    Console::print(fundsStr.str());

    Console::setCursorPosition(x + 2, y + 3);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Portfolio Value: ");
    Console::setColor(TextColor::Cyan, bodyBg);

    std::stringstream portfolioStr;
    portfolioStr << std::fixed << std::setprecision(2) << playerPtr->getPortfolio()->getTotalStocksValue() << "$";
    Console::print(portfolioStr.str());

    Console::setCursorPosition(x + 2, y + 4);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Total Asset Value: ");
    Console::setColor(TextColor::Yellow, bodyBg);

    std::stringstream assetsStr;
    assetsStr << std::fixed << std::setprecision(2) << playerPtr->getTotalAssetValue() << "$";
    Console::print(assetsStr.str());

    Console::setCursorPosition(x, y + 5);
    Console::drawHorizontalLine(x, y + 5, width);
}

void FinancialScreen::drawSectionTitle() const {
    Console::setCursorPosition(x + 2, y + 6);
    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);

    if (currentSection == FinancialSection::Loans) {
        Console::print("CURRENT OBLIGATIONS:");
    } else {
        Console::print("MARGIN ACCOUNT:");
    }

    Console::setStyle(TextStyle::Regular);
}

void FinancialScreen::drawCurrentObligations() const {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    const std::vector<Loan>& loans = playerPtr->getLoans();
    bool hasActiveLoans = false;

    for (const auto& loan : loans) {
        if (!loan.getIsPaid()) {
            hasActiveLoans = true;
            break;
        }
    }

    if (!hasActiveLoans) {
        Console::setCursorPosition(x + 4, y + 9);
        Console::setColor(TextColor::Green, bodyBg);
        Console::print("No active loans");
    } else {
        loansTable.draw();
    }
}

void FinancialScreen::drawMarginInfo() const {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    Console::setCursorPosition(x + 2, y + 8);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Current Leverage: 1: ");

    double leverage = 0.0;
    double portfolioValue = playerPtr->getPortfolio()->getTotalStocksValue();

    if (portfolioValue > 0 && playerPtr->getMarginUsed() > 0) {
        leverage = (portfolioValue + playerPtr->getMarginUsed()) / portfolioValue;
        Console::setColor(TextColor::Yellow, bodyBg);
        std::stringstream leverageStr;
        leverageStr << std::fixed << std::setprecision(1) << leverage;
        Console::print(leverageStr.str());
    } else {
        Console::setColor(TextColor::Green, bodyBg);
        Console::print("1.0");
    }

    Console::setCursorPosition(x + 2, y + 9);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Margin Requirement: ");

    std::stringstream reqStr;
    reqStr << std::fixed << std::setprecision(1) << (playerPtr->getMarginRequirement() * 100.0) << "%";
    Console::setColor(TextColor::Cyan, bodyBg);
    Console::print(reqStr.str());

    // Add maintenance requirement explanation
    Console::setCursorPosition(x + 2, y + 10);
    Console::setColor(TextColor::Yellow, bodyBg);
    Console::print("(Minimum equity that must be maintained)");

    Console::setCursorPosition(x + 2, y + 12);
    Console::setColor(TextColor::White, bodyBg);
    Console::print("Margin Account Status:");

    // Display margin account balance more clearly
    Console::setCursorPosition(x + 4, y + 13);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Balance: ");
    Console::setColor(TextColor::Green, bodyBg);
    std::stringstream balanceStr;
    balanceStr << std::fixed << std::setprecision(2) << playerPtr->getMarginAccountBalance() << "$";
    Console::print(balanceStr.str());

    // Display margin used clearly
    Console::setCursorPosition(x + 4, y + 14);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Used: ");
    if (playerPtr->getMarginUsed() > 0) {
        Console::setColor(TextColor::Red, bodyBg);
    } else {
        Console::setColor(TextColor::Green, bodyBg);
    }
    std::stringstream usedStr;
    usedStr << std::fixed << std::setprecision(2) << playerPtr->getMarginUsed() << "$";
    Console::print(usedStr.str());

    // Display available margin
    Console::setCursorPosition(x + 4, y + 15);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Available: ");
    Console::setColor(TextColor::Cyan, bodyBg);
    std::stringstream availableStr;
    availableStr << std::fixed << std::setprecision(2) << playerPtr->getMarginAvailable() << "$";
    Console::print(availableStr.str());

    // Display margin interest rate
    Console::setCursorPosition(x + 4, y + 16);
    Console::setColor(bodyFg, bodyBg);
    Console::print("Rate: ");
    Console::setColor(TextColor::Yellow, bodyBg);
    std::stringstream rateStr;
    rateStr << std::fixed << std::setprecision(1) << (playerPtr->getMarginInterestRate() * 100.0) << "%";
    Console::print(rateStr.str());

    // Margin call warning if close to maintenance requirement
    portfolioValue = playerPtr->getPortfolio()->getTotalStocksValue();
    double requiredEquity = portfolioValue * playerPtr->getMarginRequirement();
    double actualEquity = portfolioValue + playerPtr->getMarginAccountBalance() - playerPtr->getMarginUsed();

    if (actualEquity < requiredEquity * 1.2 && playerPtr->getMarginUsed() > 0) {
        Console::setCursorPosition(x + 2, y + 18);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("WARNING: Close to margin call threshold!");
    }
}
void FinancialScreen::drawAvailableLoans() const {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    Console::drawHorizontalLine(x, y + 17, width);
    Console::setCursorPosition(x + 2, y + 18);
    Console::setColor(TextColor::White, bodyBg);
    Console::print("AVAILABLE LOANS:");

    double maxLoanAmount = playerPtr->getTotalAssetValue() * 0.7;
    double currentLiabilities = playerPtr->getTotalLiabilities();
    maxLoanAmount = std::max(0.0, maxLoanAmount - currentLiabilities);

    const std::vector<Loan>& loans = playerPtr->getLoans();
    int activeLoans = 0;

    for (const auto& loan : loans) {
        if (!loan.getIsPaid()) {
            activeLoans++;
        }
    }

    struct LoanOption {
        std::string name;
        double maxAmount;
        double interestRate;
        int duration;
    };

    std::vector<LoanOption> loanOptions = {
        {"Short-term", std::min(10000.0, maxLoanAmount), 0.05, 30},
        {"Medium-term", std::min(25000.0, maxLoanAmount), 0.07, 90},
        {"Long-term", std::min(50000.0, maxLoanAmount), 0.10, 180}
    };

    bool canTakeMoreLoans = (activeLoans < MAX_LOANS);

    for (size_t i = 0; i < loanOptions.size(); i++) {
        const auto& option = loanOptions[i];

        Console::setCursorPosition(x + 1, y + 20 + static_cast<int>(i));
        Console::setColor(bodyFg, bodyBg);
        Console::print(std::to_string(i + 1) + ". " + option.name + ": ");

        std::stringstream amountStr;
        amountStr << "up to " << std::fixed << std::setprecision(2) << option.maxAmount << "$ @ ";
        amountStr << std::fixed << std::setprecision(1) << (option.interestRate * 100.0) << "% (";
        amountStr << option.duration << " days)";

        if (!canTakeMoreLoans) {
            Console::setColor(TextColor::Red, bodyBg);
            Console::print("Not available (max loans reached)");
        } else if (option.maxAmount <= 0) {
            Console::setColor(TextColor::Red, bodyBg);
            Console::print("Not available (limit reached)");
        } else {
            Console::setColor(TextColor::Green, bodyBg);
            Console::print(amountStr.str());
        }
    }
}

void FinancialScreen::drawNavigationOptions() const {
    Console::drawHorizontalLine(x, y + 24, width);

    Console::setCursorPosition(x + 1, y + 25);
    Console::setColor(bodyFg, bodyBg);
    Console::print("1. Take a Loan");

    Console::setCursorPosition(x + 1, y + 26);
    Console::print("2. Repay Loan");

    Console::setCursorPosition(x + 1, y + 27);
    Console::print("3. Manage Margin Account");

    Console::setCursorPosition(x + 1, y + 28);
    Console::print("0. Return to Main Menu");


    Console::setCursorPosition(x + 2, height - 1);
    Console::print("Choose action: ");
}

bool FinancialScreen::handleInput(int key) {
    switch (key) {
        case '1':
            takeLoan();
            return true;

        case '2':
            repayLoan();
            return true;

        case '3':
            manageMarginAccount();
            return true;

        case '0':
        case 27:
            close();
            return false;

        default:
            return Screen::handleInput(key);
    }
}

void FinancialScreen::takeLoan() {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    const std::vector<Loan>& loans = playerPtr->getLoans();
    int activeLoans = 0;

    for (const auto& loan : loans) {
        if (!loan.getIsPaid()) {
            activeLoans++;
        }
    }

    if (activeLoans >= MAX_LOANS) {
        int messageY = y + 18;
        for (int i = messageY; i < 23 ; i++) {
            Console::setCursorPosition(x, i);
            Console::print("|" + std::string(width - 2, ' ') + "|");
        }
        Console::setCursorPosition(x + 2, messageY + 1);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("You have reached the maximum of " + std::to_string(MAX_LOANS) + " loans!");

        Console::setCursorPosition(x + 2, messageY + 3);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Press any key to continue...");

        Console::readChar();
        draw();
        return;
    }

    double maxLoanAmount = playerPtr->getTotalAssetValue() * 0.7;
    double currentLiabilities = playerPtr->getTotalLiabilities();
    maxLoanAmount = std::max(0.0, maxLoanAmount - currentLiabilities);

    if (maxLoanAmount <= 0) {
        int messageY = y + 18;
        for (int i = messageY; i < 23 ; i++) {
            Console::setCursorPosition(x, i);
            Console::print("|" + std::string(width - 2, ' ') + "|");
        }
        Console::setCursorPosition(x + 4, messageY);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("Cannot take more loans! Credit limit reached.");

        Console::setCursorPosition(x + 4, messageY + 2);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Press any key to continue...");

        Console::readChar();
        draw();
        return;
    }

    displayLoanOptions();
}

void FinancialScreen::displayLoanOptions() const {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    double maxLoanAmount = playerPtr->getTotalAssetValue() * 0.7;
    double currentLiabilities = playerPtr->getTotalLiabilities();
    maxLoanAmount = std::max(0.0, maxLoanAmount - currentLiabilities);

    Console::setCursorPosition(x + 2, y + 18);
    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);
    Console::print(" SELECT LOAN TYPE:");
    Console::setStyle(TextStyle::Regular);

    struct LoanOption {
        std::string name;
        double maxAmount;
        double interestRate;
        int duration;
    };

    std::vector<LoanOption> loanOptions = {
        {"Short-term", std::min(10000.0, maxLoanAmount), 0.05, 30},
        {"Medium-term", std::min(25000.0, maxLoanAmount), 0.07, 90},
        {"Long-term", std::min(50000.0, maxLoanAmount), 0.10, 180}
    };

    std::vector<std::string> options;
    for (const auto& option : loanOptions) {
        std::stringstream optionStr;
        optionStr << option.name << ": up to " << std::fixed << std::setprecision(2)
                 << option.maxAmount << "$ @ " << std::setprecision(1) << (option.interestRate * 100.0)
                 << "% (" << option.duration << " days)";
        options.push_back(optionStr.str());
    }
    options.push_back("Cancel");

    int selected = 0;
    bool running = true;
    int menuY = y + 20;

    while (running) {
        for (size_t i = 0; i < options.size(); i++) {
            Console::setCursorPosition(x + 1, menuY + static_cast<int>(i));

            if (i == static_cast<size_t>(selected)) {
                Console::setColor(TextColor::Black, TextColor::White);
                Console::print("  >" + options[i]);
            } else {
                Console::setColor(bodyFg, bodyBg);
                Console::print("   " + options[i]);
            }

            int spaceCount = width - 7 - options[i].length();
            if (spaceCount > 0) {
                Console::print(std::string(spaceCount, ' '));
            }
        }


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
                selected = static_cast<int>(options.size()) - 1;
                running = false;
                break;

            default:
                if (key >= '1' && key <= '4' && (key - '1') < static_cast<int>(options.size())) {
                    selected = key - '1';
                    running = false;
                }
                break;
        }
    }

    if (selected < static_cast<int>(loanOptions.size())) {
        const auto& selectedOption = loanOptions[selected];

        Console::setCursorPosition(x + 2, y + 6);
        Console::setColor(TextColor::White, bodyBg);
        Console::print(std::string(width - 4, ' '));
        Console::setCursorPosition(x + 2, y + 6);
        Console::print("Enter loan amount (max " + std::to_string(static_cast<int>(selectedOption.maxAmount)) + "$): ");

        for (int i = y + 7; i < y + 20; i++) {
            Console::setCursorPosition(x, i);
            Console::print("|" + std::string(width - 2, ' ') + "|");
        }

        Console::setCursorPosition(x + 7, y + 8);
        Console::setColor(TextColor::Yellow, bodyBg);

        std::string input = Console::readLine();
        double amount = 0.0;

        try {
            amount = std::stod(input);
        } catch (...) {
            amount = 0.0;
        }

        if (amount <= 0 || amount > selectedOption.maxAmount) {
            Console::setCursorPosition(x + 4, y + 12);
            Console::setColor(TextColor::Red, bodyBg);
            Console::print("Invalid amount! Press any key to continue.");
            Console::readChar();
        } else {
            const_cast<FinancialScreen*>(this)->processTakeLoan(
                amount,
                selectedOption.interestRate,
                selectedOption.duration,
                selectedOption.name + " Loan"
            );

            Console::setCursorPosition(x + 4, y + 12);
            Console::setColor(TextColor::Green, bodyBg);
            Console::print("Loan approved! Press any key to continue.");
            Console::readChar();
        }
    }

    draw();
}

void FinancialScreen::repayLoan() {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    const std::vector<Loan>& loans = playerPtr->getLoans();
    bool hasActiveLoans = false;

    for (const auto& loan : loans) {
        if (!loan.getIsPaid()) {
            hasActiveLoans = true;
            break;
        }
    }

    if (!hasActiveLoans) {
        int messageY = y + height / 2;

        Console::setCursorPosition(x + 4, messageY - 1);
        Console::setColor(TextColor::Green, bodyBg);
        Console::print("No active loans to repay!");

        Console::setCursorPosition(x + 4, messageY + 1);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Press any key to continue.");

        Console::readChar();
        draw();
        return;
    }

    for (int i = y + 6; i < y + 24; i++) {
        Console::setCursorPosition(x, i);
        Console::setColor(bodyFg, bodyBg);
        Console::print("|" + std::string(width - 2, ' ') + "|");
    }

    Console::setCursorPosition(x + 2, y + 6);

    Console::setColor(TextColor::White, bodyBg);
    Console::setStyle(TextStyle::Bold);
    Console::print("SELECT LOAN TO REPAY:");
    Console::setStyle(TextStyle::Regular);

    std::vector<std::string> options;
    std::vector<size_t> activeLoanIndices;

    for (size_t i = 0; i < loans.size(); i++) {
        const Loan& loan = loans[i];

        if (loan.getIsPaid()) {
            continue;
        }

        std::stringstream optionStr;
        optionStr << "Loan #" << (i + 1) << ": " << std::fixed << std::setprecision(2)
                 << loan.getTotalDue() << "$ due";

        options.push_back(optionStr.str());
        activeLoanIndices.push_back(i);
    }

    options.push_back("Cancel");

    int selected = 0;
    bool running = true;
    int menuY = y + 10;

    while (running) {
        for (size_t i = 0; i < options.size(); i++) {
            Console::setCursorPosition(x + 4, menuY + static_cast<int>(i));

            if (i == static_cast<size_t>(selected)) {
                Console::setColor(TextColor::Black, TextColor::White);
                Console::print("> " + options[i]);
            } else {
                Console::setColor(bodyFg, bodyBg);
                Console::print("  " + options[i]);
            }

            int spaceCount = width - 7 - options[i].length();
            if (spaceCount > 0) {
                Console::print(std::string(spaceCount, ' '));
            }
        }

        Console::setCursorPosition(x + 8, y + 13);
        Console::setColor(bodyFg, bodyBg);

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

        }
    }

    if (selected < static_cast<int>(activeLoanIndices.size())) {
        size_t loanIndex = activeLoanIndices[selected];
        const Loan& loan = loans[loanIndex];
        double totalDue = loan.getTotalDue();

        if (playerPtr->getPortfolio()->getCashBalance() < totalDue) {
            Console::setCursorPosition(x + 4, y + 20);
            Console::setColor(TextColor::Red, bodyBg);
            Console::print("Insufficient funds to repay this loan!");

            Console::setCursorPosition(x + 4, y + 22);
            Console::setColor(bodyFg, bodyBg);
            Console::print("Press any key to continue.");

            Console::readChar();
        } else {
            processLoanRepayment(loanIndex);

            Console::setCursorPosition(x + 4, y + 20);
            Console::setColor(TextColor::Green, bodyBg);
            Console::print("Loan repaid successfully!");

            Console::setCursorPosition(x + 4, y + 22);
            Console::setColor(bodyFg, bodyBg);
            Console::print("Press any key to continue.");

            Console::readChar();
        }
    }

    draw();
}

void FinancialScreen::manageMarginAccount() {
    currentSection = FinancialSection::MarginAccount;
    draw();

    for (int i = y + 20; i < y + 23; i++) {
        Console::setCursorPosition(x, i);
        Console::setColor(bodyFg, bodyBg);
        Console::print("|" + std::string(width - 2, ' ') + "|");
    }

    std::vector<std::string> options = {
        "Deposit to Margin Account",
        "Withdraw from Margin Account",
        "Return to Loans"
    };

    int selected = 0;
    bool running = true;
    int menuY = y + 18;

    while (running) {
        updateMarginTable();

        for (size_t i = 0; i < options.size(); i++) {
            Console::setCursorPosition(x + 2, menuY + static_cast<int>(i));

            if (i == static_cast<size_t>(selected)) {
                Console::setColor(TextColor::Black, TextColor::White);
                Console::print("> " + options[i]);
            } else {
                Console::setColor(bodyFg, bodyBg);
                Console::print("  " + options[i]);
            }

            int spaceCount = width - 7 - options[i].length();
            if (spaceCount > 0) {
                Console::print(std::string(spaceCount, ' '));
            }
        }

        Console::setCursorPosition(x + 4, menuY + static_cast<int>(options.size()) + 1);
        Console::setColor(bodyFg, bodyBg);

        char key = Console::readChar();

        switch (key) {
            case static_cast<char>(Key::ArrowUp):
                selected = (selected > 0) ? selected - 1 : static_cast<int>(options.size()) - 1;
                break;

            case static_cast<char>(Key::ArrowDown):
                selected = (selected < static_cast<int>(options.size()) - 1) ? selected + 1 : 0;
                break;

            case static_cast<char>(Key::Enter):
                switch (selected) {
                    case 0:
                        depositToMargin();
                        break;

                    case 1:
                        withdrawFromMargin();
                        break;

                    case 2:
                        currentSection = FinancialSection::Loans;
                        running = false;
                        break;
                }
                draw();
                break;

            case static_cast<char>(Key::Escape):
                currentSection = FinancialSection::Loans;
                running = false;
                break;
        }
    }

    draw();
}
void FinancialScreen::depositToMargin() {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    double availableCash = playerPtr->getPortfolio()->getCashBalance();

    if (availableCash <= 0) {
        Console::setCursorPosition(x + 4, y + 22);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("No cash available to deposit!");

        Console::setCursorPosition(x + 4, y + 23);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Press any key to continue.");

        Console::readChar();
        return;
    }

    Console::setCursorPosition(x + 4, y + 22);
    Console::setColor(TextColor::White, bodyBg);
    Console::print("Enter amount to deposit (max " +
                  std::to_string(static_cast<int>(availableCash)) + "$): ");

    Console::setCursorPosition(x + 4, y + 23);
    Console::setColor(TextColor::Yellow, bodyBg);

    std::string input = Console::readLine();
    double amount = 0.0;

    try {
        amount = std::stod(input);
    } catch (...) {
        amount = 0.0;
    }

    if (amount <= 0 || amount > availableCash) {
        Console::setCursorPosition(x + 4, y + 23);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("Invalid amount! Press any key to continue.");
        Console::readChar();
    } else {
        bool success = playerPtr->depositToMarginAccount(amount);

        Console::setCursorPosition(x + 4, y + 23);

        if (success) {
            Console::setColor(TextColor::Green, bodyBg);
            Console::print("Deposit successful! Press any key to continue.");

            update();
        } else {
            Console::setColor(TextColor::Red, bodyBg);
            Console::print("Deposit failed! Press any key to continue.");
        }

        Console::readChar();
    }
}

void FinancialScreen::withdrawFromMargin() {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    double marginBalance = playerPtr->getMarginAccountBalance();

    if (marginBalance <= 0) {
        // Show error message
        Console::setCursorPosition(x + 4, y + 22);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("No funds in margin account to withdraw!");

        Console::setCursorPosition(x + 4, y + 24);
        Console::setColor(bodyFg, bodyBg);
        Console::print("Press any key to continue.");

        Console::readChar();
        return;
    }

    Console::setCursorPosition(x + 4, y + 22);
    Console::setColor(TextColor::White, bodyBg);
    Console::print("Enter amount to withdraw (max " +
                  std::to_string(static_cast<int>(marginBalance)) + "$): ");

    Console::setCursorPosition(x + 4, y + 23);
    Console::setColor(TextColor::Yellow, bodyBg);

    std::string input = Console::readLine();
    double amount = 0.0;

    try {
        amount = std::stod(input);
    } catch (...) {
        amount = 0.0;
    }

    if (amount <= 0 || amount > marginBalance) {
        Console::setCursorPosition(x + 4, y + 23);
        Console::setColor(TextColor::Red, bodyBg);
        Console::print("Invalid amount! Press any key to continue.");
        Console::readChar();
    } else {
        bool success = playerPtr->withdrawFromMarginAccount(amount);

        Console::setCursorPosition(x + 4, y + 23);

        if (success) {
            Console::setColor(TextColor::Green, bodyBg);
            Console::print("Successful! Press any key to continue.");

            update();
        } else {
            Console::setColor(TextColor::Red, bodyBg);
            Console::print("Failed! Press any key to continue.");
        }

        Console::readChar();
    }
}

FinancialSection FinancialScreen::getCurrentSection() const {
    return currentSection;
}

void FinancialScreen::setCurrentSection(FinancialSection section) {
    currentSection = section;
}

int FinancialScreen::getSelectedLoanIndex() const {
    return selectedLoanIndex;
}

void FinancialScreen::setSelectedLoanIndex(int index) {
    selectedLoanIndex = index;
}

void FinancialScreen::processLoanRepayment(size_t loanIndex) {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    const std::vector<Loan>& loans = playerPtr->getLoans();

    if (loanIndex >= loans.size()) {
        return;
    }

    const Loan& loan = loans[loanIndex];
    double totalDue = loan.getTotalDue();

    playerPtr->repayLoan(loanIndex, totalDue);
    update();
}

void FinancialScreen::processTakeLoan(double amount, double interestRate, int durationDays, const std::string& description) {
    auto playerPtr = player.lock();
    if (!playerPtr) {
        return;
    }

    playerPtr->takeLoan(amount, interestRate, durationDays, description);
    update();
}

}