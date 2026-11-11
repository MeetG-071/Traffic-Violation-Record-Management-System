#include "ui.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "date_utils.h"
#include "report.h"
#include "validation.h"

#define MAX_RECORD_ID INT_MAX

static void printError(Status_t status)
{
    printf("Error: %s\n", statusToString(status));
}

static void toUpperCase(char *text)
{
    for (; *text != '\0'; ++text)
    {
        *text = (char)toupper((unsigned char)*text);
    }
}

/* Reads a line and copies it into 'destination' only when it fits. */
static Status_t promptField(const char *prompt, char *destination, size_t destinationSize)
{
    char buffer[INPUT_BUFFER_SIZE];
    Status_t status = readLine(prompt, buffer, sizeof(buffer));

    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    if (strlen(buffer) >= destinationSize)
    {
        return STATUS_INVALID_INPUT;
    }
    snprintf(destination, destinationSize, "%s", buffer);
    return STATUS_SUCCESS;
}

static int confirm(const char *prompt)
{
    char answer[INPUT_BUFFER_SIZE];

    if (readLine(prompt, answer, sizeof(answer)) != STATUS_SUCCESS)
    {
        return 0;
    }
    return (answer[0] == 'y' || answer[0] == 'Y') && answer[1] == '\0';
}

Status_t readLine(const char *prompt, char *buffer, size_t size)
{
    size_t length;
    int ch;

    if (prompt == NULL || buffer == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    if (size < 2)
    {
        return STATUS_INVALID_ARGUMENT;
    }
    printf("%s", prompt);
    fflush(stdout);

    if (fgets(buffer, (int)size, stdin) == NULL)
    {
        buffer[0] = '\0';
        return STATUS_END_OF_INPUT;
    }
    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n')
    {
        buffer[--length] = '\0';
    }
    else if (!feof(stdin))
    {
        while ((ch = getchar()) != '\n' && ch != EOF)
        {
            /* discard the rest of an over-long line */
        }
        return STATUS_INVALID_INPUT;
    }
    if (length > 0 && buffer[length - 1] == '\r')
    {
        buffer[length - 1] = '\0';
    }
    return STATUS_SUCCESS;
}

Status_t readIntegerInRange(const char *prompt, int min, int max, int *value)
{
    char buffer[INPUT_BUFFER_SIZE];
    Status_t status = readLine(prompt, buffer, sizeof(buffer));

    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    return parseInteger(buffer, min, max, value);
}

void displayMainMenu(void)
{
    printf("\n===== Traffic Violation Record Management System =====\n");
    printf(" 1. Add violation record\n");
    printf(" 2. List all records\n");
    printf(" 3. Search record by ID\n");
    printf(" 4. Vehicle violation history\n");
    printf(" 5. Mark fine as paid\n");
    printf(" 6. Delete record\n");
    printf(" 7. Show overdue fines\n");
    printf(" 8. Show records sorted by fine\n");
    printf(" 9. Summary report\n");
    printf(" 0. Exit\n");
}

int handleAddRecord(RecordStore_t *store)
{
    ViolationRecord_t record;
    char today[DATE_STRING_LEN + 1];
    int choice = 0;
    int newId = 0;
    int type;
    Status_t status;

    memset(&record, 0, sizeof(record));

    status = promptField("Vehicle number (e.g. MH12AB1234): ", record.vehicleNumber,
                         sizeof(record.vehicleNumber));
    if (status != STATUS_SUCCESS) { printError(status); return 0; }
    toUpperCase(record.vehicleNumber);

    status = promptField("Driver name: ", record.driverName, sizeof(record.driverName));
    if (status != STATUS_SUCCESS) { printError(status); return 0; }

    status = promptField("Location: ", record.location, sizeof(record.location));
    if (status != STATUS_SUCCESS) { printError(status); return 0; }

    status = promptField("Date YYYY-MM-DD (Enter for today): ", record.date, sizeof(record.date));
    if (status != STATUS_SUCCESS) { printError(status); return 0; }
    if (record.date[0] == '\0')
    {
        status = getTodayDate(today, sizeof(today));
        if (status != STATUS_SUCCESS) { printError(status); return 0; }
        snprintf(record.date, sizeof(record.date), "%s", today);
    }

    printf("Violation types:\n");
    for (type = 0; type < (int)VIOLATION_TYPE_COUNT; ++type)
    {
        printf("  %d. %s\n", type + 1, violationTypeToString((ViolationType_t)type));
    }
    status = readIntegerInRange("Select type: ", 1, (int)VIOLATION_TYPE_COUNT, &choice);
    if (status != STATUS_SUCCESS) { printError(status); return 0; }
    record.type = (ViolationType_t)(choice - 1);

    if (record.type == VIOLATION_SPEEDING)
    {
        status = readIntegerInRange("Recorded speed (km/h): ", MIN_SPEED_KMH, MAX_SPEED_KMH,
                                    &record.recordedSpeed);
        if (status != STATUS_SUCCESS) { printError(status); return 0; }
        status = readIntegerInRange("Speed limit (km/h): ", MIN_SPEED_KMH, MAX_SPEED_KMH,
                                    &record.speedLimit);
        if (status != STATUS_SUCCESS) { printError(status); return 0; }
    }

    status = addRecord(store, &record, &newId);
    if (status != STATUS_SUCCESS)
    {
        printError(status);
        return 0;
    }
    printf("Record added. ID: %d, Fine: Rs %.2f\n", newId, store->records[store->count - 1].fineAmount);
    return 1;
}

void handleListRecords(const RecordStore_t *store)
{
    if (store->count == 0)
    {
        printError(STATUS_EMPTY_STORE);
        return;
    }
    printRecordTable(stdout, store->records, store->count);
}

void handleSearchById(const RecordStore_t *store)
{
    int id = 0;
    size_t index = 0;
    Status_t status = readIntegerInRange("Record ID: ", 1, MAX_RECORD_ID, &id);

    if (status == STATUS_SUCCESS)
    {
        status = findRecordIndexById(store, id, &index);
    }
    if (status != STATUS_SUCCESS)
    {
        printError(status);
        return;
    }
    printRecordDetails(stdout, &store->records[index]);
}

void handleVehicleHistory(const RecordStore_t *store)
{
    char vehicle[VEHICLE_NUMBER_MAX_LEN + 1];
    ViolationRecord_t *matches = NULL;
    size_t count = 0;
    size_t i;
    double total = 0.0;
    Status_t status = promptField("Vehicle number: ", vehicle, sizeof(vehicle));

    if (status == STATUS_SUCCESS)
    {
        toUpperCase(vehicle);
        status = validateVehicleNumber(vehicle);
    }
    if (status == STATUS_SUCCESS)
    {
        status = collectVehicleRecords(store, vehicle, &matches, &count);
    }
    if (status != STATUS_SUCCESS)
    {
        printError(status);
        return;
    }
    printRecordTable(stdout, matches, count);
    for (i = 0; i < count; ++i)
    {
        total += matches[i].fineAmount;
    }
    printf("%zu violation(s), total fines Rs %.2f\n", count, total);
    free(matches);
}

int handleMarkPaid(RecordStore_t *store)
{
    int id = 0;
    Status_t status = readIntegerInRange("Record ID: ", 1, MAX_RECORD_ID, &id);

    if (status == STATUS_SUCCESS)
    {
        status = markRecordPaid(store, id);
    }
    if (status != STATUS_SUCCESS)
    {
        printError(status);
        return 0;
    }
    printf("Record %d marked as paid.\n", id);
    return 1;
}

int handleDeleteRecord(RecordStore_t *store)
{
    int id = 0;
    size_t index = 0;
    Status_t status = readIntegerInRange("Record ID to delete: ", 1, MAX_RECORD_ID, &id);

    if (status == STATUS_SUCCESS)
    {
        status = findRecordIndexById(store, id, &index);
    }
    if (status != STATUS_SUCCESS)
    {
        printError(status);
        return 0;
    }
    if (!confirm("Type Y to confirm deletion: "))
    {
        printf("Deletion cancelled.\n");
        return 0;
    }
    status = deleteRecord(store, id);
    if (status != STATUS_SUCCESS)
    {
        printError(status);
        return 0;
    }
    printf("Record %d deleted.\n", id);
    return 1;
}

void handleOverdueReport(const RecordStore_t *store)
{
    char today[DATE_STRING_LEN + 1];
    ViolationRecord_t *overdue = NULL;
    size_t count = 0;
    Status_t status = getTodayDate(today, sizeof(today));

    if (status == STATUS_SUCCESS)
    {
        status = collectOverdueRecords(store, today, &overdue, &count);
    }
    if (status == STATUS_NOT_FOUND)
    {
        printf("No overdue fines (due period: %d days).\n", PAYMENT_DUE_DAYS);
        return;
    }
    if (status != STATUS_SUCCESS)
    {
        printError(status);
        return;
    }
    printf("Overdue fines as of %s (unpaid for more than %d days):\n", today, PAYMENT_DUE_DAYS);
    printRecordTable(stdout, overdue, count);
    free(overdue);
}

void handleSortedByFine(const RecordStore_t *store)
{
    ViolationRecord_t *sorted = NULL;
    size_t count = 0;
    Status_t status = buildSortedByFine(store, &sorted, &count);

    if (status != STATUS_SUCCESS)
    {
        printError(status);
        return;
    }
    printRecordTable(stdout, sorted, count);
    free(sorted);
}

void handleSummaryReport(const RecordStore_t *store)
{
    ReportSummary_t summary;
    char today[DATE_STRING_LEN + 1];
    Status_t status = computeSummary(store, &summary);

    if (status != STATUS_SUCCESS)
    {
        printError(status);
        return;
    }
    printSummaryReport(stdout, &summary);

    if (confirm("Export this report to " SUMMARY_REPORT_FILE "? (y/n): "))
    {
        status = getTodayDate(today, sizeof(today));
        if (status == STATUS_SUCCESS)
        {
            status = writeSummaryReport(store, SUMMARY_REPORT_FILE, today);
        }
        if (status == STATUS_SUCCESS)
        {
            printf("Report saved to %s\n", SUMMARY_REPORT_FILE);
        }
        else
        {
            printError(status);
        }
    }
}
