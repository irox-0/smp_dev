#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace StockMarketSimulator {

    class Date {
    private:
        int day;
        int month;
        int year;

        void normalize();
        static const std::vector<int> daysInMonth;

        int toAbsoluteDayNumber() const;

    public:
        Date(int d = 1, int m = 3, int y = 2023);

        int getDay() const;
        int getMonth() const;
        int getYear() const;

        void setDay(int d);
        void setMonth(int m);
        void setYear(int y);

        void nextDay();
        void advanceDays(int days);

        std::string toString() const;
        std::string toShortString() const;

        bool operator==(const Date& other) const;
        bool operator!=(const Date& other) const;
        bool operator<(const Date& other) const;
        bool operator>(const Date& other) const;
        bool operator<=(const Date& other) const;
        bool operator>=(const Date& other) const;

        int daysBetween(const Date& other) const;
        bool isLeapYear(int y) const;
        int getDaysInMonth(int m, int y) const;

        static Date fromDayNumber(int dayNumber, const Date& startDate = Date(1, 3, 2023));
        int toDayNumber(const Date& startDate = Date(1, 3, 2023)) const;

        nlohmann::json toJson() const;
        static Date fromJson(const nlohmann::json& json);
    };

}