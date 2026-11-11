#include <gtest/gtest.h>

#include "config.h"
#include "date_utils.h"

TEST(LeapYearTest, FollowsGregorianRules)
{
    EXPECT_TRUE(isLeapYear(2024));
    EXPECT_FALSE(isLeapYear(2023));
    EXPECT_FALSE(isLeapYear(2100));
    EXPECT_TRUE(isLeapYear(2000));
}

TEST(DaysInMonthTest, ReturnsCorrectDaysAndZeroForInvalidMonth)
{
    EXPECT_EQ(31, daysInMonth(2026, 1));
    EXPECT_EQ(28, daysInMonth(2026, 2));
    EXPECT_EQ(29, daysInMonth(2024, 2));
    EXPECT_EQ(30, daysInMonth(2026, 11));
    EXPECT_EQ(0, daysInMonth(2026, 0));
    EXPECT_EQ(0, daysInMonth(2026, 13));
}

TEST(ParseDateTest, ExtractsFields)
{
    int y = 0, m = 0, d = 0;
    ASSERT_EQ(STATUS_SUCCESS, parseDate("2026-11-11", &y, &m, &d));
    EXPECT_EQ(2026, y);
    EXPECT_EQ(11, m);
    EXPECT_EQ(11, d);
}

TEST(ParseDateTest, RejectsNullArguments)
{
    int y = 0, m = 0, d = 0;
    EXPECT_EQ(STATUS_NULL_POINTER, parseDate(nullptr, &y, &m, &d));
    EXPECT_EQ(STATUS_NULL_POINTER, parseDate("2026-11-11", nullptr, &m, &d));
}

TEST(DaysBetweenDatesTest, CountsWithinMonth)
{
    long diff = 0;
    ASSERT_EQ(STATUS_SUCCESS, daysBetweenDates("2024-01-01", "2024-01-31", &diff));
    EXPECT_EQ(30, diff);
}

TEST(DaysBetweenDatesTest, HandlesLeapDayAndYearEnd)
{
    long diff = 0;
    ASSERT_EQ(STATUS_SUCCESS, daysBetweenDates("2024-02-28", "2024-03-01", &diff));
    EXPECT_EQ(2, diff);
    ASSERT_EQ(STATUS_SUCCESS, daysBetweenDates("2025-12-31", "2026-01-01", &diff));
    EXPECT_EQ(1, diff);
}

TEST(DaysBetweenDatesTest, ReturnsZeroAndNegativeValues)
{
    long diff = 99;
    ASSERT_EQ(STATUS_SUCCESS, daysBetweenDates("2026-05-05", "2026-05-05", &diff));
    EXPECT_EQ(0, diff);
    ASSERT_EQ(STATUS_SUCCESS, daysBetweenDates("2026-05-10", "2026-05-05", &diff));
    EXPECT_EQ(-5, diff);
}

TEST(DaysBetweenDatesTest, RejectsInvalidDate)
{
    long diff = 0;
    EXPECT_EQ(STATUS_INVALID_DATE, daysBetweenDates("2026-02-30", "2026-03-01", &diff));
}

TEST(TodayDateTest, ProducesValidDateAndRejectsSmallBuffer)
{
    char buffer[DATE_STRING_LEN + 1];
    char tiny[5];
    int y = 0, m = 0, d = 0;
    ASSERT_EQ(STATUS_SUCCESS, getTodayDate(buffer, sizeof(buffer)));
    EXPECT_EQ(STATUS_SUCCESS, parseDate(buffer, &y, &m, &d));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, getTodayDate(tiny, sizeof(tiny)));
}
