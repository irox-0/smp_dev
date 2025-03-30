#include "Date.hpp"
#include "Date.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

namespace StockMarketSimulator {

const std::vector<int> Date::daysInMonth = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

Date::Date(int d, int m, int y) : day(d), month(m), year(y) {
    normalize();
}

void Date::normalize() {
    while (month > 12) {
        month -= 12;
        year++;
    }

    while (month < 1) {
        month += 12;
        year--;
    }

    int maxDays = getDaysInMonth(month, year);

    while (day > maxDays) {
        day -= maxDays;
        month++;

        if (month > 12) {
            month = 1;
            year++;
        }

        maxDays = getDaysInMonth(month, year);
    }

    while (day < 1) {
        month--;

        if (month < 1) {
            month = 12;
            year--;
        }

        maxDays = getDaysInMonth(month, year);
        day += maxDays;
    }
}

int Date::getDay() const {
    return day;
}

int Date::getMonth() const {
    return month;
}

int Date::getYear() const {
    return year;
}

void Date::setDay(int d) {
    day = d;
    normalize();
}

void Date::setMonth(int m) {
    month = m;
    normalize();
}

void Date::setYear(int y) {
    year = y;
    normalize();
}

void Date::nextDay() {
    day++;
    normalize();
}

void Date::advanceDays(int days) {
    day += days;
    normalize();
}

std::string Date::toString() const {
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << day << "."
       << std::setfill('0') << std::setw(2) << month << "."
       << year;
    return ss.str();
}

std::string Date::toShortString() const {
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << day << "/"
       << std::setfill('0') << std::setw(2) << month;
    return ss.str();
}

bool Date::operator==(const Date& other) const {
    return day == other.day && month == other.month && year == other.year;
}

bool Date::operator!=(const Date& other) const {
    return !(*this == other);
}

bool Date::operator<(const Date& other) const {
    if (year != other.year) return year < other.year;
    if (month != other.month) return month < other.month;
    return day < other.day;
}

bool Date::operator>(const Date& other) const {
    return other < *this;
}

bool Date::operator<=(const Date& other) const {
    return *this < other || *this == other;
}

bool Date::operator>=(const Date& other) const {
    return *this > other || *this == other;
}

int Date::daysBetween(const Date& other) const {
    Date refDate(1, 1, 2000);
    int thisDayNum = this->toDayNumber(refDate);
    int otherDayNum = other.toDayNumber(refDate);

    return otherDayNum - thisDayNum;
}

bool Date::isLeapYear(int y) const {
    return (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
}

int Date::getDaysInMonth(int m, int y) const {
    if (m == 2 && isLeapYear(y)) {
        return 29;
    }
    return daysInMonth[m];
}

Date Date::fromDayNumber(int dayNumber, const Date& startDate) {
    Date result = startDate;
    result.advanceDays(dayNumber - 1);
    return result;
}

int Date::toDayNumber(const Date& startDate) const {
    if (*this < startDate) {
        return -startDate.daysBetween(*this);
    }

    int days = 0;
    Date temp = startDate;

    while (temp.year < year) {
        days += isLeapYear(temp.year) ? 366 : 365;
        temp.year++;
    }

    while (temp.month < month) {
        days += getDaysInMonth(temp.month, temp.year);
        temp.month++;
    }

    days += day - temp.day;

    return days + 1; // +1 because day 1 is startDate
}

nlohmann::json Date::toJson() const {
    nlohmann::json j;
    j["day"] = day;
    j["month"] = month;
    j["year"] = year;
    return j;
}

Date Date::fromJson(const nlohmann::json& json) {
    return Date(json["day"], json["month"], json["year"]);
}

} // namespace StockMarketSimulator