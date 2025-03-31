#include <gtest/gtest.h>
#include "../../src/utils/Date.hpp"
#include <sstream>
#include <vector>

using namespace StockMarketSimulator;

class DateTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
};

TEST_F(DateTest, ConstructorAndGetters) {
    Date defaultDate;
    EXPECT_EQ(defaultDate.getDay(), 1);
    EXPECT_EQ(defaultDate.getMonth(), 3);
    EXPECT_EQ(defaultDate.getYear(), 2023);

    Date customDate(15, 7, 2022);
    EXPECT_EQ(customDate.getDay(), 15);
    EXPECT_EQ(customDate.getMonth(), 7);
    EXPECT_EQ(customDate.getYear(), 2022);
}

TEST_F(DateTest, Setters) {
    Date date(1, 1, 2023);

    date.setDay(15);
    EXPECT_EQ(date.getDay(), 15);

    date.setMonth(6);
    EXPECT_EQ(date.getMonth(), 6);

    date.setYear(2024);
    EXPECT_EQ(date.getYear(), 2024);
}

TEST_F(DateTest, DateNormalization) {
    Date date1(32, 1, 2023);
    EXPECT_EQ(date1.getDay(), 1);
    EXPECT_EQ(date1.getMonth(), 2);

    Date date2(1, 13, 2023);
    EXPECT_EQ(date2.getDay(), 1);
    EXPECT_EQ(date2.getMonth(), 1);
    EXPECT_EQ(date2.getYear(), 2024);

    Date date3(0, 3, 2023);
    EXPECT_EQ(date3.getDay(), 28);
    EXPECT_EQ(date3.getMonth(), 2);

    Date date4(1, 0, 2023);
    EXPECT_EQ(date4.getDay(), 1);
    EXPECT_EQ(date4.getMonth(), 12);
    EXPECT_EQ(date4.getYear(), 2022);

    Date date5(60, 1, 2023);
    EXPECT_EQ(date5.getMonth(), 3);
    EXPECT_EQ(date5.getDay(), 1);
}

TEST_F(DateTest, LeapYearHandling) {
    Date leapYear(29, 2, 2020);
    EXPECT_EQ(leapYear.getDay(), 29);
    EXPECT_EQ(leapYear.getMonth(), 2);

    Date afterLeapDay(30, 2, 2020);
    EXPECT_EQ(afterLeapDay.getDay(), 1);
    EXPECT_EQ(afterLeapDay.getMonth(), 3);

    Date nonLeapYear(29, 2, 2023);
    EXPECT_EQ(nonLeapYear.getDay(), 1);
    EXPECT_EQ(nonLeapYear.getMonth(), 3);

    Date testDate;
    EXPECT_TRUE(testDate.isLeapYear(2020));
    EXPECT_TRUE(testDate.isLeapYear(2000));
    EXPECT_FALSE(testDate.isLeapYear(2023));
    EXPECT_FALSE(testDate.isLeapYear(1900));
}

TEST_F(DateTest, ComparisonOperators) {
    Date earlier(1, 1, 2023);
    Date later(2, 1, 2023);
    Date same(1, 1, 2023);
    Date differentMonth(1, 2, 2023);
    Date differentYear(1, 1, 2024);

    EXPECT_TRUE(earlier == same);
    EXPECT_FALSE(earlier == later);

    EXPECT_TRUE(earlier != later);
    EXPECT_FALSE(earlier != same);

    EXPECT_TRUE(earlier < later);
    EXPECT_TRUE(earlier < differentMonth);
    EXPECT_TRUE(earlier < differentYear);
    EXPECT_FALSE(later < earlier);

    EXPECT_TRUE(later > earlier);
    EXPECT_TRUE(differentMonth > earlier);
    EXPECT_TRUE(differentYear > earlier);
    EXPECT_FALSE(earlier > later);

    EXPECT_TRUE(earlier <= same);
    EXPECT_TRUE(earlier <= later);
    EXPECT_FALSE(later <= earlier);

    EXPECT_TRUE(later >= earlier);
    EXPECT_TRUE(same >= earlier);
    EXPECT_FALSE(earlier >= later);
}

TEST_F(DateTest, DateAdvancement) {
    Date date(15, 3, 2023);

    date.nextDay();
    EXPECT_EQ(date.getDay(), 16);
    EXPECT_EQ(date.getMonth(), 3);
    EXPECT_EQ(date.getYear(), 2023);

    date.advanceDays(20);
    EXPECT_EQ(date.getDay(), 5);
    EXPECT_EQ(date.getMonth(), 4);
    EXPECT_EQ(date.getYear(), 2023);

    Date monthEnd(30, 4, 2023);
    monthEnd.nextDay();
    EXPECT_EQ(monthEnd.getDay(), 1);
    EXPECT_EQ(monthEnd.getMonth(), 5);

    Date yearEnd(31, 12, 2023);
    yearEnd.nextDay();
    EXPECT_EQ(yearEnd.getDay(), 1);
    EXPECT_EQ(yearEnd.getMonth(), 1);
    EXPECT_EQ(yearEnd.getYear(), 2024);

    Date startDate(1, 1, 2023);
    startDate.advanceDays(365);
    EXPECT_EQ(startDate.getDay(), 1);
    EXPECT_EQ(startDate.getMonth(), 1);
    EXPECT_EQ(startDate.getYear(), 2024);
}

TEST_F(DateTest, DaysBetween) {
    Date earlier(1, 1, 2023);
    Date later(1, 2, 2023);
    Date muchLater(1, 1, 2024);

    EXPECT_EQ(earlier.daysBetween(later), 31);
    EXPECT_EQ(earlier.daysBetween(muchLater), 365);
    EXPECT_EQ(later.daysBetween(earlier), -31);
    EXPECT_EQ(earlier.daysBetween(earlier), 0);
}

TEST_F(DateTest, StringFormatting) {
    Date date(5, 7, 2023);

    EXPECT_EQ(date.toString(), "05.07.2023");

    EXPECT_EQ(date.toShortString(), "05/07");

    Date singleDigits(1, 3, 2023);
    EXPECT_EQ(singleDigits.toString(), "01.03.2023");
    EXPECT_EQ(singleDigits.toShortString(), "01/03");
}

TEST_F(DateTest, DayNumberConversion) {
    Date referenceDate(1, 3, 2023);

    int dayNum1 = referenceDate.toDayNumber(referenceDate);
    EXPECT_EQ(dayNum1, 1);

    Date threeDaysLater(4, 3, 2023);
    int dayNum2 = threeDaysLater.toDayNumber(referenceDate);
    EXPECT_EQ(dayNum2, 4);

    Date previousDay(28, 2, 2023);
    int dayNum3 = previousDay.toDayNumber(referenceDate);
    EXPECT_EQ(dayNum3, 2);

    Date day1 = Date::fromDayNumber(1, referenceDate);
    EXPECT_EQ(day1.getDay(), 1);
    EXPECT_EQ(day1.getMonth(), 3);
    EXPECT_EQ(day1.getYear(), 2023);

    Date day31 = Date::fromDayNumber(31, referenceDate);
    EXPECT_EQ(day31.getDay(), 31);
    EXPECT_EQ(day31.getMonth(), 3);
    EXPECT_EQ(day31.getYear(), 2023);

    Date day32 = Date::fromDayNumber(32, referenceDate);
    EXPECT_EQ(day32.getDay(), 1);
    EXPECT_EQ(day32.getMonth(), 4);
    EXPECT_EQ(day32.getYear(), 2023);

    Date day366 = Date::fromDayNumber(366, referenceDate);
    EXPECT_EQ(day366.getDay(), 29);
    EXPECT_EQ(day366.getMonth(), 2);
    EXPECT_EQ(day366.getYear(), 2024);

    Date dayNegative30 = Date::fromDayNumber(-30, referenceDate);
    EXPECT_EQ(dayNegative30.getDay(), 29);
    EXPECT_EQ(dayNegative30.getMonth(), 1);
    EXPECT_EQ(dayNegative30.getYear(), 2023);

    Date dayNegative1 = Date::fromDayNumber(-1, referenceDate);
    EXPECT_EQ(dayNegative1.getDay(), 27);
    EXPECT_EQ(dayNegative1.getMonth(), 2);
    EXPECT_EQ(dayNegative1.getYear(), 2023);

    Date dayNegative2 = Date::fromDayNumber(-2, referenceDate);
    EXPECT_EQ(dayNegative2.getDay(), 26);
    EXPECT_EQ(dayNegative2.getMonth(), 2);
    EXPECT_EQ(dayNegative2.getYear(), 2023);
}

TEST_F(DateTest, JsonSerialization) {
    Date original(15, 7, 2023);
    nlohmann::json json = original.toJson();

    EXPECT_EQ(json["day"], 15);
    EXPECT_EQ(json["month"], 7);
    EXPECT_EQ(json["year"], 2023);

    Date deserialized = Date::fromJson(json);

    EXPECT_EQ(deserialized.getDay(), 15);
    EXPECT_EQ(deserialized.getMonth(), 7);
    EXPECT_EQ(deserialized.getYear(), 2023);
    EXPECT_TRUE(original == deserialized);
}

TEST_F(DateTest, EdgeCases) {
    Date farFuture(1, 1, 9999);
    EXPECT_EQ(farFuture.getYear(), 9999);

    std::vector<int> daysInMonth = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (int month = 1; month <= 12; month++) {
        Date lastDay(daysInMonth[month], month, 2023);
        EXPECT_EQ(lastDay.getDay(), daysInMonth[month]);
        EXPECT_EQ(lastDay.getMonth(), month);

        Date dayAfter(daysInMonth[month] + 1, month, 2023);
        EXPECT_EQ(dayAfter.getDay(), 1);
        EXPECT_EQ(dayAfter.getMonth(), month < 12 ? month + 1 : 1);
        EXPECT_EQ(dayAfter.getYear(), month < 12 ? 2023 : 2024);
    }

    Date feb29_2020(29, 2, 2020);
    EXPECT_EQ(feb29_2020.getDay(), 29);
    EXPECT_EQ(feb29_2020.getMonth(), 2);

    Date feb29_2021(29, 2, 2021);
    EXPECT_EQ(feb29_2021.getDay(), 1);
    EXPECT_EQ(feb29_2021.getMonth(), 3);
}

TEST_F(DateTest, GetDaysInMonth) {
    Date date;

    EXPECT_EQ(date.getDaysInMonth(1, 2023), 31);
    EXPECT_EQ(date.getDaysInMonth(4, 2023), 30);

    EXPECT_EQ(date.getDaysInMonth(2, 2020), 29);
    EXPECT_EQ(date.getDaysInMonth(2, 2023), 28);

    std::vector<int> expectedDays = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (int month = 1; month <= 12; month++) {
        EXPECT_EQ(date.getDaysInMonth(month, 2023), expectedDays[month]);
    }
}
