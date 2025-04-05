#pragma once

#include "../Screen.hpp"
#include "../../core/Player.hpp"
#include "../../ui/widgets/Table.hpp"
#include "../../ui/widgets/Menu.hpp"
#include <vector>
#include <memory>

namespace StockMarketSimulator {

    enum class FinancialSection {
        Loans,
        MarginAccount
    };

    class FinancialScreen : public Screen {
    private:
        FinancialSection currentSection;
        Table loansTable;
        Table marginTable;
        int selectedLoanIndex;

        void updateLoansTable();
        void updateMarginTable();
        void takeLoan();
        void repayLoan();
        void manageMarginAccount();
        void displayLoanOptions() const;

        // New methods for simplified margin trading
        void takeMarginLoan();
        void repayMarginLoan();

        void drawFinancialInfo() const;
        void drawCurrentObligations() const;
        void drawMarginInfo() const;
        void drawAvailableLoans() const;
        void drawNavigationOptions() const;
        void drawSectionTitle() const;

    protected:
        virtual void drawContent() const override;

    public:
        FinancialScreen();

        virtual void initialize() override;
        virtual void update() override;
        virtual bool handleInput(int key) override;

        FinancialSection getCurrentSection() const;
        void setCurrentSection(FinancialSection section);

        int getSelectedLoanIndex() const;
        void setSelectedLoanIndex(int index);

        void processLoanRepayment(size_t loanIndex);
        void processTakeLoan(double amount, double interestRate, int durationDays, const std::string& description);
    };

}