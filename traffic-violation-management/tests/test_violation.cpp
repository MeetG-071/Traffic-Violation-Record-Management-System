#include <gtest/gtest.h>

#include <cstdio>

#include "test_helpers.hpp"
#include "violation.h"

class RecordStoreTest : public ::testing::Test
{
protected:
    void SetUp() override { initRecordStore(&store); }
    RecordStore_t store;
};

/* ---- calculateFine ---- */

TEST(CalculateFineTest, SpeedingFineGrowsWithExcessSpeed)
{
    double fine = 0.0;
    ASSERT_EQ(STATUS_SUCCESS, calculateFine(VIOLATION_SPEEDING, 80, 60, 0, &fine));
    EXPECT_DOUBLE_EQ(3000.0, fine);
}

TEST(CalculateFineTest, FixedFinesPerViolationType)
{
    double fine = 0.0;
    ASSERT_EQ(STATUS_SUCCESS, calculateFine(VIOLATION_RED_LIGHT, 0, 0, 0, &fine));
    EXPECT_DOUBLE_EQ(FINE_RED_LIGHT, fine);
    ASSERT_EQ(STATUS_SUCCESS, calculateFine(VIOLATION_NO_HELMET, 0, 0, 0, &fine));
    EXPECT_DOUBLE_EQ(FINE_NO_HELMET, fine);
    ASSERT_EQ(STATUS_SUCCESS, calculateFine(VIOLATION_ILLEGAL_PARKING, 0, 0, 0, &fine));
    EXPECT_DOUBLE_EQ(FINE_ILLEGAL_PARKING, fine);
}

TEST(CalculateFineTest, RepeatOffenderPenaltyStartsAtThreshold)
{
    double fine = 0.0;
    ASSERT_EQ(STATUS_SUCCESS, calculateFine(VIOLATION_RED_LIGHT, 0, 0, REPEAT_OFFENDER_THRESHOLD - 1, &fine));
    EXPECT_DOUBLE_EQ(FINE_RED_LIGHT, fine);
    ASSERT_EQ(STATUS_SUCCESS, calculateFine(VIOLATION_RED_LIGHT, 0, 0, REPEAT_OFFENDER_THRESHOLD, &fine));
    EXPECT_DOUBLE_EQ(FINE_RED_LIGHT * REPEAT_OFFENDER_MULTIPLIER, fine);
}

TEST(CalculateFineTest, SpeedingAtMaximumSpeedStaysWithinFineCap)
{
    double fine = 0.0;
    ASSERT_EQ(STATUS_SUCCESS, calculateFine(VIOLATION_SPEEDING, MAX_SPEED_KMH, 1, 10, &fine));
    EXPECT_LE(fine, MAX_FINE_AMOUNT);
}

TEST(CalculateFineTest, RejectsInvalidArguments)
{
    double fine = 0.0;
    EXPECT_EQ(STATUS_INVALID_SPEED, calculateFine(VIOLATION_SPEEDING, 60, 60, 0, &fine));
    EXPECT_EQ(STATUS_INVALID_TYPE, calculateFine(static_cast<ViolationType_t>(42), 0, 0, 0, &fine));
    EXPECT_EQ(STATUS_NULL_POINTER, calculateFine(VIOLATION_RED_LIGHT, 0, 0, 0, nullptr));
}

/* ---- addRecord ---- */

TEST_F(RecordStoreTest, AddAssignsIdFineAndUnpaidStatus)
{
    ViolationRecord_t record = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_SPEEDING);
    int id = 0;
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &record, &id));
    EXPECT_EQ(1, id);
    ASSERT_EQ(1u, store.count);
    EXPECT_DOUBLE_EQ(3000.0, store.records[0].fineAmount);
    EXPECT_EQ(PAYMENT_UNPAID, store.records[0].paymentStatus);
}

TEST_F(RecordStoreTest, AddGeneratesIncreasingIds)
{
    int first = 0, second = 0;
    ViolationRecord_t a = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_NO_HELMET);
    ViolationRecord_t b = makeRecord("MH14CD5678", "2026-11-01", VIOLATION_NO_HELMET);
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &a, &first));
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &b, &second));
    EXPECT_EQ(first + 1, second);
}

TEST_F(RecordStoreTest, AddRejectsInvalidVehicleAndKeepsStoreUnchanged)
{
    ViolationRecord_t record = makeRecord("bad", "2026-11-01", VIOLATION_RED_LIGHT);
    int id = 0;
    EXPECT_EQ(STATUS_INVALID_VEHICLE_NUMBER, addRecord(&store, &record, &id));
    EXPECT_EQ(0u, store.count);
}

TEST_F(RecordStoreTest, AddRejectsDuplicateRecord)
{
    ViolationRecord_t record = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_RED_LIGHT);
    int id = 0;
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &record, &id));
    EXPECT_EQ(STATUS_DUPLICATE, addRecord(&store, &record, &id));
    EXPECT_EQ(1u, store.count);
}

TEST_F(RecordStoreTest, AddAppliesRepeatOffenderPenaltyOnFourthViolation)
{
    const char *dates[] = {"2026-11-01", "2026-11-02", "2026-11-03", "2026-11-04"};
    int id = 0;
    for (const char *date : dates)
    {
        ViolationRecord_t record = makeRecord("MH12AB1234", date, VIOLATION_RED_LIGHT);
        ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &record, &id));
    }
    EXPECT_DOUBLE_EQ(FINE_RED_LIGHT, store.records[2].fineAmount);
    EXPECT_DOUBLE_EQ(FINE_RED_LIGHT * REPEAT_OFFENDER_MULTIPLIER, store.records[3].fineAmount);
}

TEST_F(RecordStoreTest, AddFailsWhenStorageIsFull)
{
    int id = 0;
    for (int i = 0; i < MAX_RECORDS; ++i)
    {
        char vehicle[VEHICLE_NUMBER_MAX_LEN + 1];
        std::snprintf(vehicle, sizeof(vehicle), "MH12AB%04d", i);
        ViolationRecord_t record = makeRecord(vehicle, "2026-11-01", VIOLATION_NO_HELMET);
        ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &record, &id));
    }
    ViolationRecord_t extra = makeRecord("KA01ZZ9999", "2026-11-01", VIOLATION_NO_HELMET);
    EXPECT_EQ(STATUS_STORAGE_FULL, addRecord(&store, &extra, &id));
    EXPECT_EQ(static_cast<size_t>(MAX_RECORDS), store.count);
}

TEST_F(RecordStoreTest, AddRejectsNullArguments)
{
    ViolationRecord_t record = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_RED_LIGHT);
    int id = 0;
    EXPECT_EQ(STATUS_NULL_POINTER, addRecord(nullptr, &record, &id));
    EXPECT_EQ(STATUS_NULL_POINTER, addRecord(&store, nullptr, &id));
    EXPECT_EQ(STATUS_NULL_POINTER, addRecord(&store, &record, nullptr));
}

/* ---- find / delete / pay ---- */

TEST_F(RecordStoreTest, FindReportsEmptyStoreAndMissingId)
{
    size_t index = 0;
    EXPECT_EQ(STATUS_EMPTY_STORE, findRecordIndexById(&store, 1, &index));

    ViolationRecord_t record = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_RED_LIGHT);
    int id = 0;
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &record, &id));
    EXPECT_EQ(STATUS_SUCCESS, findRecordIndexById(&store, id, &index));
    EXPECT_EQ(0u, index);
    EXPECT_EQ(STATUS_NOT_FOUND, findRecordIndexById(&store, 999, &index));
}

TEST_F(RecordStoreTest, DeleteRemovesRecordAndShiftsOthers)
{
    int a = 0, b = 0, c = 0;
    ViolationRecord_t r1 = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_NO_HELMET);
    ViolationRecord_t r2 = makeRecord("MH14CD5678", "2026-11-01", VIOLATION_NO_HELMET);
    ViolationRecord_t r3 = makeRecord("MH20EF0001", "2026-11-01", VIOLATION_NO_HELMET);
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &r1, &a));
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &r2, &b));
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &r3, &c));

    ASSERT_EQ(STATUS_SUCCESS, deleteRecord(&store, b));
    EXPECT_EQ(2u, store.count);
    EXPECT_EQ(a, store.records[0].id);
    EXPECT_EQ(c, store.records[1].id);
}

TEST_F(RecordStoreTest, DeleteMissingRecordFails)
{
    EXPECT_EQ(STATUS_EMPTY_STORE, deleteRecord(&store, 1));
    ViolationRecord_t record = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_RED_LIGHT);
    int id = 0;
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &record, &id));
    EXPECT_EQ(STATUS_NOT_FOUND, deleteRecord(&store, id + 10));
    EXPECT_EQ(1u, store.count);
}

TEST_F(RecordStoreTest, MarkPaidChangesStatusOnlyOnce)
{
    ViolationRecord_t record = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_RED_LIGHT);
    int id = 0;
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &record, &id));
    EXPECT_EQ(STATUS_SUCCESS, markRecordPaid(&store, id));
    EXPECT_EQ(PAYMENT_PAID, store.records[0].paymentStatus);
    EXPECT_EQ(STATUS_ALREADY_PAID, markRecordPaid(&store, id));
    EXPECT_EQ(STATUS_NOT_FOUND, markRecordPaid(&store, id + 5));
}

TEST_F(RecordStoreTest, CountViolationsByVehicleCountsOnlyMatches)
{
    int id = 0;
    ViolationRecord_t a = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_RED_LIGHT);
    ViolationRecord_t b = makeRecord("MH12AB1234", "2026-11-02", VIOLATION_RED_LIGHT);
    ViolationRecord_t c = makeRecord("MH14CD5678", "2026-11-01", VIOLATION_RED_LIGHT);
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &a, &id));
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &b, &id));
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &c, &id));
    EXPECT_EQ(2u, countViolationsByVehicle(&store, "MH12AB1234"));
    EXPECT_EQ(0u, countViolationsByVehicle(&store, "UNKNOWN123"));
    EXPECT_EQ(0u, countViolationsByVehicle(&store, nullptr));
}

/* ---- overdue status ---- */

TEST(OverdueTest, ExactlyDueDaysIsNotOverdueButOneMoreDayIs)
{
    ViolationRecord_t record = makeRecord("MH12AB1234", "2024-01-01", VIOLATION_RED_LIGHT);
    int overdue = -1;
    ASSERT_EQ(STATUS_SUCCESS, isRecordOverdue(&record, "2024-01-31", &overdue));
    EXPECT_EQ(0, overdue);
    ASSERT_EQ(STATUS_SUCCESS, isRecordOverdue(&record, "2024-02-01", &overdue));
    EXPECT_EQ(1, overdue);
}

TEST(OverdueTest, PaidRecordIsNeverOverdue)
{
    ViolationRecord_t record = makeRecord("MH12AB1234", "2024-01-01", VIOLATION_RED_LIGHT);
    record.paymentStatus = PAYMENT_PAID;
    int overdue = -1;
    ASSERT_EQ(STATUS_SUCCESS, isRecordOverdue(&record, "2026-01-01", &overdue));
    EXPECT_EQ(0, overdue);
}

TEST(OverdueTest, RejectsInvalidTodayAndNulls)
{
    ViolationRecord_t record = makeRecord("MH12AB1234", "2024-01-01", VIOLATION_RED_LIGHT);
    int overdue = 0;
    EXPECT_EQ(STATUS_INVALID_DATE, isRecordOverdue(&record, "not-a-date", &overdue));
    EXPECT_EQ(STATUS_NULL_POINTER, isRecordOverdue(nullptr, "2024-01-01", &overdue));
}
