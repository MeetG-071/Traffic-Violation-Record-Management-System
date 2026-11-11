#include <gtest/gtest.h>

#include <string>

#include "test_helpers.hpp"
#include "validation.h"

TEST(VehicleNumberTest, AcceptsTypicalNumber)
{
    EXPECT_EQ(STATUS_SUCCESS, validateVehicleNumber("MH12AB1234"));
}

TEST(VehicleNumberTest, AcceptsMinimumLengthBoundary)
{
    EXPECT_EQ(STATUS_SUCCESS, validateVehicleNumber("MH1234"));
}

TEST(VehicleNumberTest, RejectsBelowMinimumLength)
{
    EXPECT_EQ(STATUS_INVALID_VEHICLE_NUMBER, validateVehicleNumber("MH123"));
}

TEST(VehicleNumberTest, RejectsAboveMaximumLength)
{
    EXPECT_EQ(STATUS_INVALID_VEHICLE_NUMBER, validateVehicleNumber("MH12AB12345"));
}

TEST(VehicleNumberTest, RejectsLowercaseAndSymbols)
{
    EXPECT_EQ(STATUS_INVALID_VEHICLE_NUMBER, validateVehicleNumber("mh12ab1234"));
    EXPECT_EQ(STATUS_INVALID_VEHICLE_NUMBER, validateVehicleNumber("MH12-B1234"));
}

TEST(VehicleNumberTest, RejectsNullAndEmpty)
{
    EXPECT_EQ(STATUS_NULL_POINTER, validateVehicleNumber(nullptr));
    EXPECT_EQ(STATUS_INVALID_VEHICLE_NUMBER, validateVehicleNumber(""));
}

TEST(NameTest, AcceptsLettersSpacesAndPunctuation)
{
    EXPECT_EQ(STATUS_SUCCESS, validateName("Mary-Ann O'Neil Jr."));
}

TEST(NameTest, RejectsDigitsAndEmpty)
{
    EXPECT_EQ(STATUS_INVALID_NAME, validateName("R2D2"));
    EXPECT_EQ(STATUS_INVALID_NAME, validateName(""));
    EXPECT_EQ(STATUS_INVALID_NAME, validateName("   "));
}

TEST(NameTest, EnforcesMaximumLengthBoundary)
{
    EXPECT_EQ(STATUS_SUCCESS, validateName(std::string(DRIVER_NAME_MAX_LEN, 'A').c_str()));
    EXPECT_EQ(STATUS_INVALID_NAME, validateName(std::string(DRIVER_NAME_MAX_LEN + 1, 'A').c_str()));
}

TEST(LocationTest, RejectsFileDelimiterAndBlank)
{
    EXPECT_EQ(STATUS_SUCCESS, validateLocation("MG Road, Pune"));
    EXPECT_EQ(STATUS_INVALID_LOCATION, validateLocation("MG|Road"));
    EXPECT_EQ(STATUS_INVALID_LOCATION, validateLocation("    "));
}

TEST(DateTest, AcceptsValidDatesIncludingLeapDay)
{
    EXPECT_EQ(STATUS_SUCCESS, validateDate("2026-11-11"));
    EXPECT_EQ(STATUS_SUCCESS, validateDate("2024-02-29"));
}

TEST(DateTest, RejectsNonLeapFebruary29)
{
    EXPECT_EQ(STATUS_INVALID_DATE, validateDate("2023-02-29"));
}

TEST(DateTest, RejectsBadFormatAndMonthAndDay)
{
    EXPECT_EQ(STATUS_INVALID_DATE, validateDate("2024/01/01"));
    EXPECT_EQ(STATUS_INVALID_DATE, validateDate("2024-13-01"));
    EXPECT_EQ(STATUS_INVALID_DATE, validateDate("2024-04-31"));
    EXPECT_EQ(STATUS_INVALID_DATE, validateDate("2024-1-01"));
    EXPECT_EQ(STATUS_NULL_POINTER, validateDate(nullptr));
}

TEST(DateTest, EnforcesYearBoundaries)
{
    EXPECT_EQ(STATUS_SUCCESS, validateDate("2000-01-01"));
    EXPECT_EQ(STATUS_INVALID_DATE, validateDate("1999-12-31"));
    EXPECT_EQ(STATUS_SUCCESS, validateDate("2100-12-31"));
    EXPECT_EQ(STATUS_INVALID_DATE, validateDate("2101-01-01"));
}

TEST(SpeedTest, AcceptsSpeedAboveLimit)
{
    EXPECT_EQ(STATUS_SUCCESS, validateSpeedPair(61, 60));
}

TEST(SpeedTest, RejectsSpeedEqualOrBelowLimit)
{
    EXPECT_EQ(STATUS_INVALID_SPEED, validateSpeedPair(60, 60));
    EXPECT_EQ(STATUS_INVALID_SPEED, validateSpeedPair(50, 60));
}

TEST(SpeedTest, EnforcesMaximumBoundary)
{
    EXPECT_EQ(STATUS_SUCCESS, validateSpeedPair(MAX_SPEED_KMH, 60));
    EXPECT_EQ(STATUS_INVALID_SPEED, validateSpeedPair(MAX_SPEED_KMH + 1, 60));
}

TEST(ParseIntegerTest, ParsesValidNumbersAtBoundaries)
{
    int value = 0;
    EXPECT_EQ(STATUS_SUCCESS, parseInteger("1", 1, 9, &value));
    EXPECT_EQ(1, value);
    EXPECT_EQ(STATUS_SUCCESS, parseInteger("9", 1, 9, &value));
    EXPECT_EQ(9, value);
}

TEST(ParseIntegerTest, RejectsOutOfRangeAndGarbage)
{
    int value = 123;
    EXPECT_EQ(STATUS_INVALID_INPUT, parseInteger("0", 1, 9, &value));
    EXPECT_EQ(STATUS_INVALID_INPUT, parseInteger("10", 1, 9, &value));
    EXPECT_EQ(STATUS_INVALID_INPUT, parseInteger("5x", 1, 9, &value));
    EXPECT_EQ(STATUS_INVALID_INPUT, parseInteger("", 1, 9, &value));
    EXPECT_EQ(STATUS_INVALID_INPUT, parseInteger(" 5", 1, 9, &value));
    EXPECT_EQ(STATUS_INVALID_INPUT, parseInteger("99999999999999999999", 1, 9, &value));
    EXPECT_EQ(123, value);
}

TEST(ParseDoubleTest, ParsesAndRejects)
{
    double value = 0.0;
    EXPECT_EQ(STATUS_SUCCESS, parseDouble("3200.50", &value));
    EXPECT_DOUBLE_EQ(3200.5, value);
    EXPECT_EQ(STATUS_INVALID_INPUT, parseDouble("abc", &value));
    EXPECT_EQ(STATUS_INVALID_INPUT, parseDouble("nan", &value));
}

TEST(FineAmountTest, EnforcesBoundaries)
{
    EXPECT_EQ(STATUS_SUCCESS, validateFineAmount(0.01));
    EXPECT_EQ(STATUS_SUCCESS, validateFineAmount(MAX_FINE_AMOUNT));
    EXPECT_EQ(STATUS_INVALID_FINE, validateFineAmount(0.0));
    EXPECT_EQ(STATUS_INVALID_FINE, validateFineAmount(MAX_FINE_AMOUNT + 1.0));
}

TEST(RecordFieldsTest, AcceptsValidSpeedingRecord)
{
    ViolationRecord_t record = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_SPEEDING);
    EXPECT_EQ(STATUS_SUCCESS, validateRecordFields(&record));
}

TEST(RecordFieldsTest, RejectsSpeedValuesOnNonSpeedingRecord)
{
    ViolationRecord_t record = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_RED_LIGHT);
    record.recordedSpeed = 70;
    EXPECT_EQ(STATUS_INVALID_SPEED, validateRecordFields(&record));
}

TEST(RecordFieldsTest, RejectsInvalidTypeAndNull)
{
    ViolationRecord_t record = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_RED_LIGHT);
    record.type = static_cast<ViolationType_t>(99);
    EXPECT_EQ(STATUS_INVALID_TYPE, validateRecordFields(&record));
    EXPECT_EQ(STATUS_NULL_POINTER, validateRecordFields(nullptr));
}
