#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>

#include "file_handler.h"
#include "test_helpers.hpp"

class FileHandlerTest : public ::testing::Test
{
protected:
    static constexpr const char *kPath = "test_tmp_violations.txt";

    void SetUp() override
    {
        std::remove(kPath);
        initRecordStore(&store);
    }
    void TearDown() override { std::remove(kPath); }

    void writeFile(const std::string &content)
    {
        std::ofstream out(kPath);
        out << content;
    }

    RecordStore_t store;
};

TEST_F(FileHandlerTest, SaveThenLoadRoundTripsRecords)
{
    int id = 0;
    ViolationRecord_t a = makeRecord("MH12AB1234", "2026-11-01", VIOLATION_SPEEDING);
    ViolationRecord_t b = makeRecord("MH14CD5678", "2026-11-02", VIOLATION_NO_SEATBELT);
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &a, &id));
    ASSERT_EQ(STATUS_SUCCESS, addRecord(&store, &b, &id));
    ASSERT_EQ(STATUS_SUCCESS, markRecordPaid(&store, id));
    ASSERT_EQ(STATUS_SUCCESS, saveRecordsToFile(&store, kPath));

    RecordStore_t loaded;
    ASSERT_EQ(STATUS_SUCCESS, loadRecordsFromFile(&loaded, kPath));
    ASSERT_EQ(2u, loaded.count);
    EXPECT_STREQ("MH12AB1234", loaded.records[0].vehicleNumber);
    EXPECT_EQ(80, loaded.records[0].recordedSpeed);
    EXPECT_DOUBLE_EQ(3000.0, loaded.records[0].fineAmount);
    EXPECT_EQ(PAYMENT_PAID, loaded.records[1].paymentStatus);
    EXPECT_EQ(3, loaded.nextId);
}

TEST_F(FileHandlerTest, LoadMissingFileReportsNotFoundAndLeavesStoreEmpty)
{
    EXPECT_EQ(STATUS_FILE_NOT_FOUND, loadRecordsFromFile(&store, kPath));
    EXPECT_EQ(0u, store.count);
}

TEST_F(FileHandlerTest, LoadSkipsCommentsAndBlankLines)
{
    writeFile("# header comment\n\n1|MH12AB1234|Rahul Patil|FC Road|2026-11-01|1|0|0|1000.00|0\n\n");
    ASSERT_EQ(STATUS_SUCCESS, loadRecordsFromFile(&store, kPath));
    EXPECT_EQ(1u, store.count);
}

TEST_F(FileHandlerTest, LoadAcceptsLastLineWithoutNewlineAndWindowsLineEndings)
{
    writeFile("1|MH12AB1234|Rahul Patil|FC Road|2026-11-01|1|0|0|1000.00|0\r\n"
              "2|MH14CD5678|Sneha Kulkarni|Wakad|2026-11-02|2|0|0|500.00|1");
    ASSERT_EQ(STATUS_SUCCESS, loadRecordsFromFile(&store, kPath));
    EXPECT_EQ(2u, store.count);
}

TEST_F(FileHandlerTest, LoadRejectsWrongFieldCount)
{
    writeFile("1|MH12AB1234|Rahul Patil|FC Road|2026-11-01|1|0|0|1000.00\n");
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, loadRecordsFromFile(&store, kPath));
    EXPECT_EQ(0u, store.count);
}

TEST_F(FileHandlerTest, LoadRejectsInvalidFieldValues)
{
    writeFile("1|mh12ab1234|Rahul Patil|FC Road|2026-11-01|1|0|0|1000.00|0\n");
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, loadRecordsFromFile(&store, kPath));
    writeFile("1|MH12AB1234|Rahul Patil|FC Road|2026-11-01|9|0|0|1000.00|0\n");
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, loadRecordsFromFile(&store, kPath));
    writeFile("1|MH12AB1234|Rahul Patil|FC Road|2026-11-01|1|0|0|-5.00|0\n");
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, loadRecordsFromFile(&store, kPath));
}

TEST_F(FileHandlerTest, LoadRejectsDuplicateIdsAndDiscardsEarlierRecords)
{
    writeFile("1|MH12AB1234|Rahul Patil|FC Road|2026-11-01|1|0|0|1000.00|0\n"
              "1|MH14CD5678|Sneha Kulkarni|Wakad|2026-11-02|2|0|0|500.00|1\n");
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, loadRecordsFromFile(&store, kPath));
    EXPECT_EQ(0u, store.count);
}

TEST_F(FileHandlerTest, LoadRejectsOverlongLine)
{
    writeFile(std::string(LINE_BUFFER_SIZE * 2, 'x') + "\n");
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, loadRecordsFromFile(&store, kPath));
}

TEST_F(FileHandlerTest, SaveToUnwritablePathFails)
{
    EXPECT_EQ(STATUS_FILE_OPEN_FAILED, saveRecordsToFile(&store, "no_such_directory/out.txt"));
}

TEST_F(FileHandlerTest, NullArgumentsAreRejected)
{
    EXPECT_EQ(STATUS_NULL_POINTER, loadRecordsFromFile(nullptr, kPath));
    EXPECT_EQ(STATUS_NULL_POINTER, loadRecordsFromFile(&store, nullptr));
    EXPECT_EQ(STATUS_NULL_POINTER, saveRecordsToFile(nullptr, kPath));
    EXPECT_EQ(STATUS_NULL_POINTER, saveRecordsToFile(&store, nullptr));
}
