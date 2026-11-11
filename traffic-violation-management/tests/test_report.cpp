#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#include "report.h"
#include "test_helpers.hpp"

class ReportTest : public ::testing::Test
{
protected:
    void SetUp() override { initRecordStore(&store); }
    void TearDown() override { std::remove("test_tmp_report.txt"); }

    void add(const char *vehicle, const char *date, ViolationType_t type)
    {
        ViolationRecord_t record = makeRecord(vehicle, date, type);
        int id = 0;
        ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &record, &id));
    }

    RecordStore_t store;
};

TEST_F(ReportTest, SummaryOfEmptyStoreReportsEmpty)
{
    ReportSummary_t summary;
    EXPECT_EQ(STATUS_EMPTY_STORE, computeSummary(&store, &summary));
    EXPECT_EQ(0u, summary.totalRecords);
}

TEST_F(ReportTest, SummaryTotalsFinesAndPaymentSplit)
{
    add("MH12AB1234", "2026-11-01", VIOLATION_RED_LIGHT);   /* 1000 */
    add("MH14CD5678", "2026-11-01", VIOLATION_NO_HELMET);   /* 500  */
    add("MH20EF0001", "2026-11-01", VIOLATION_SPEEDING);    /* 3000 */
    ASSERT_EQ(STATUS_SUCCESS, markRecordPaid(&store, 2));

    ReportSummary_t summary;
    ASSERT_EQ(STATUS_SUCCESS, computeSummary(&store, &summary));
    EXPECT_EQ(3u, summary.totalRecords);
    EXPECT_EQ(2u, summary.unpaidRecords);
    EXPECT_DOUBLE_EQ(4500.0, summary.totalFines);
    EXPECT_DOUBLE_EQ(500.0, summary.collectedFines);
    EXPECT_DOUBLE_EQ(4000.0, summary.outstandingFines);
    EXPECT_EQ(1u, summary.typeCounts[VIOLATION_SPEEDING]);
    EXPECT_EQ(0u, summary.typeCounts[VIOLATION_NO_SEATBELT]);
}

TEST_F(ReportTest, SortPutsHighestFineFirstAndKeepsIdOrderOnTies)
{
    add("MH12AB1234", "2026-11-01", VIOLATION_NO_HELMET);   /* id1 500  */
    add("MH14CD5678", "2026-11-01", VIOLATION_SPEEDING);    /* id2 3000 */
    add("MH20EF0001", "2026-11-01", VIOLATION_NO_HELMET);   /* id3 500  */

    ViolationRecord_t *sorted = nullptr;
    size_t count = 0;
    ASSERT_EQ(STATUS_SUCCESS, buildSortedByFine(&store, &sorted, &count));
    ASSERT_EQ(3u, count);
    EXPECT_EQ(2, sorted[0].id);
    EXPECT_EQ(1, sorted[1].id);
    EXPECT_EQ(3, sorted[2].id);
    std::free(sorted);
}

TEST_F(ReportTest, SortedBuildOnEmptyStoreAllocatesNothing)
{
    ViolationRecord_t *sorted = nullptr;
    size_t count = 5;
    EXPECT_EQ(STATUS_EMPTY_STORE, buildSortedByFine(&store, &sorted, &count));
    EXPECT_EQ(nullptr, sorted);
    EXPECT_EQ(0u, count);
}

TEST_F(ReportTest, SortHandlesNullAndSingleElement)
{
    EXPECT_EQ(STATUS_NULL_POINTER, sortRecordsByFineDescending(nullptr, 3));
    ViolationRecord_t one = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_NO_HELMET);
    EXPECT_EQ(STATUS_SUCCESS, sortRecordsByFineDescending(&one, 1));
}

TEST_F(ReportTest, VehicleHistoryReturnsOnlyThatVehicle)
{
    add("MH12AB1234", "2026-11-01", VIOLATION_RED_LIGHT);
    add("MH14CD5678", "2026-11-01", VIOLATION_RED_LIGHT);
    add("MH12AB1234", "2026-11-02", VIOLATION_NO_HELMET);

    ViolationRecord_t *matches = nullptr;
    size_t count = 0;
    ASSERT_EQ(STATUS_SUCCESS, collectVehicleRecords(&store, "MH12AB1234", &matches, &count));
    EXPECT_EQ(2u, count);
    std::free(matches);

    EXPECT_EQ(STATUS_NOT_FOUND, collectVehicleRecords(&store, "UNKNOWN123", &matches, &count));
    EXPECT_EQ(nullptr, matches);
}

TEST_F(ReportTest, OverdueCollectionUsesDueDateBoundary)
{
    add("MH12AB1234", "2026-10-01", VIOLATION_RED_LIGHT);   /* 31 days on 2026-11-01: overdue    */
    add("MH14CD5678", "2026-10-02", VIOLATION_RED_LIGHT);   /* 30 days: not overdue              */
    add("MH20EF0001", "2026-09-01", VIOLATION_RED_LIGHT);   /* old but paid below                */
    ASSERT_EQ(STATUS_SUCCESS, markRecordPaid(&store, 3));

    size_t counted = 0;
    ASSERT_EQ(STATUS_SUCCESS, countOverdueRecords(&store, "2026-11-01", &counted));
    EXPECT_EQ(1u, counted);

    ViolationRecord_t *overdue = nullptr;
    size_t count = 0;
    ASSERT_EQ(STATUS_SUCCESS, collectOverdueRecords(&store, "2026-11-01", &overdue, &count));
    ASSERT_EQ(1u, count);
    EXPECT_EQ(1, overdue[0].id);
    std::free(overdue);
}

TEST_F(ReportTest, OverdueCollectionReportsNotFoundWhenNoneOverdue)
{
    add("MH12AB1234", "2026-11-01", VIOLATION_RED_LIGHT);
    ViolationRecord_t *overdue = nullptr;
    size_t count = 0;
    EXPECT_EQ(STATUS_NOT_FOUND, collectOverdueRecords(&store, "2026-11-05", &overdue, &count));
    EXPECT_EQ(nullptr, overdue);
}

TEST_F(ReportTest, WriteSummaryReportCreatesFileWithTotals)
{
    add("MH12AB1234", "2026-11-01", VIOLATION_RED_LIGHT);
    ASSERT_EQ(STATUS_SUCCESS, writeSummaryReport(&store, "test_tmp_report.txt", "2026-11-11"));

    std::ifstream in("test_tmp_report.txt");
    ASSERT_TRUE(in.is_open());
    std::stringstream content;
    content << in.rdbuf();
    EXPECT_NE(std::string::npos, content.str().find("Total records     : 1"));
    EXPECT_NE(std::string::npos, content.str().find("2026-11-11"));
}

TEST_F(ReportTest, WriteSummaryReportFailsForEmptyStoreOrBadPath)
{
    EXPECT_EQ(STATUS_EMPTY_STORE, writeSummaryReport(&store, "test_tmp_report.txt", "2026-11-11"));
    add("MH12AB1234", "2026-11-01", VIOLATION_RED_LIGHT);
    EXPECT_EQ(STATUS_FILE_OPEN_FAILED, writeSummaryReport(&store, "no_such_dir/r.txt", "2026-11-11"));
}
