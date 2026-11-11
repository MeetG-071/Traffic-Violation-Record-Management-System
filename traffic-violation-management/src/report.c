#include "report.h"

#include <stdlib.h>
#include <string.h>

#define LOCATION_COLUMN_WIDTH 22

static int compareByFineDescending(const void *left, const void *right)
{
    const ViolationRecord_t *a = (const ViolationRecord_t *)left;
    const ViolationRecord_t *b = (const ViolationRecord_t *)right;

    if (a->fineAmount > b->fineAmount) { return -1; }
    if (a->fineAmount < b->fineAmount) { return 1; }
    return (a->id > b->id) - (a->id < b->id);
}

static Status_t allocateRecordArray(const RecordStore_t *store, ViolationRecord_t **out)
{
    *out = (ViolationRecord_t *)malloc(store->count * sizeof(ViolationRecord_t));
    return (*out == NULL) ? STATUS_MEMORY_FAILURE : STATUS_SUCCESS;
}

Status_t computeSummary(const RecordStore_t *store, ReportSummary_t *summary)
{
    size_t i;

    if (store == NULL || summary == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    memset(summary, 0, sizeof(*summary));
    if (store->count == 0)
    {
        return STATUS_EMPTY_STORE;
    }
    for (i = 0; i < store->count; ++i)
    {
        const ViolationRecord_t *r = &store->records[i];

        summary->totalRecords++;
        summary->totalFines += r->fineAmount;
        if (r->paymentStatus == PAYMENT_PAID)
        {
            summary->collectedFines += r->fineAmount;
        }
        else
        {
            summary->unpaidRecords++;
            summary->outstandingFines += r->fineAmount;
        }
        if ((int)r->type >= 0 && r->type < VIOLATION_TYPE_COUNT)
        {
            summary->typeCounts[r->type]++;
        }
    }
    return STATUS_SUCCESS;
}

Status_t sortRecordsByFineDescending(ViolationRecord_t *records, size_t count)
{
    if (records == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    if (count > 1)
    {
        qsort(records, count, sizeof(records[0]), compareByFineDescending);
    }
    return STATUS_SUCCESS;
}

Status_t buildSortedByFine(const RecordStore_t *store, ViolationRecord_t **out, size_t *count)
{
    Status_t status;

    if (store == NULL || out == NULL || count == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    *out = NULL;
    *count = 0;
    if (store->count == 0)
    {
        return STATUS_EMPTY_STORE;
    }
    status = allocateRecordArray(store, out);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    memcpy(*out, store->records, store->count * sizeof(ViolationRecord_t));
    *count = store->count;
    return sortRecordsByFineDescending(*out, *count);
}

Status_t collectVehicleRecords(const RecordStore_t *store, const char *vehicleNumber,
                               ViolationRecord_t **out, size_t *count)
{
    size_t i;
    size_t matches;
    Status_t status;

    if (store == NULL || vehicleNumber == NULL || out == NULL || count == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    *out = NULL;
    *count = 0;
    matches = countViolationsByVehicle(store, vehicleNumber);
    if (matches == 0)
    {
        return STATUS_NOT_FOUND;
    }
    status = allocateRecordArray(store, out);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    for (i = 0; i < store->count; ++i)
    {
        if (strcmp(store->records[i].vehicleNumber, vehicleNumber) == 0)
        {
            (*out)[(*count)++] = store->records[i];
        }
    }
    return STATUS_SUCCESS;
}

Status_t countOverdueRecords(const RecordStore_t *store, const char *today, size_t *count)
{
    size_t i;
    int overdue = 0;
    Status_t status;

    if (store == NULL || today == NULL || count == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    *count = 0;
    for (i = 0; i < store->count; ++i)
    {
        status = isRecordOverdue(&store->records[i], today, &overdue);
        if (status != STATUS_SUCCESS)
        {
            return status;
        }
        if (overdue)
        {
            (*count)++;
        }
    }
    return STATUS_SUCCESS;
}

Status_t collectOverdueRecords(const RecordStore_t *store, const char *today,
                               ViolationRecord_t **out, size_t *count)
{
    size_t i;
    size_t overdueCount = 0;
    int overdue = 0;
    Status_t status;

    if (store == NULL || today == NULL || out == NULL || count == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    *out = NULL;
    *count = 0;
    status = countOverdueRecords(store, today, &overdueCount);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    if (overdueCount == 0)
    {
        return STATUS_NOT_FOUND;
    }
    status = allocateRecordArray(store, out);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    for (i = 0; i < store->count; ++i)
    {
        if (isRecordOverdue(&store->records[i], today, &overdue) == STATUS_SUCCESS && overdue)
        {
            (*out)[(*count)++] = store->records[i];
        }
    }
    return STATUS_SUCCESS;
}

void printRecordTable(FILE *out, const ViolationRecord_t *records, size_t count)
{
    size_t i;

    if (out == NULL || records == NULL)
    {
        return;
    }
    fprintf(out, "%-5s %-11s %-10s %-16s %-*s %10s  %s\n", "ID", "Vehicle", "Date", "Violation",
            LOCATION_COLUMN_WIDTH, "Location", "Fine (Rs)", "Status");
    for (i = 0; i < count; ++i)
    {
        const ViolationRecord_t *r = &records[i];
        fprintf(out, "%-5d %-11s %-10s %-16s %-*.*s %10.2f  %s\n", r->id, r->vehicleNumber,
                r->date, violationTypeToString(r->type), LOCATION_COLUMN_WIDTH,
                LOCATION_COLUMN_WIDTH, r->location, r->fineAmount,
                paymentStatusToString(r->paymentStatus));
    }
}

void printRecordDetails(FILE *out, const ViolationRecord_t *record)
{
    if (out == NULL || record == NULL)
    {
        return;
    }
    fprintf(out, "Record ID      : %d\n", record->id);
    fprintf(out, "Vehicle number : %s\n", record->vehicleNumber);
    fprintf(out, "Driver name    : %s\n", record->driverName);
    fprintf(out, "Location       : %s\n", record->location);
    fprintf(out, "Date           : %s\n", record->date);
    fprintf(out, "Violation      : %s\n", violationTypeToString(record->type));
    if (record->type == VIOLATION_SPEEDING)
    {
        fprintf(out, "Speed / limit  : %d / %d km/h\n", record->recordedSpeed, record->speedLimit);
    }
    fprintf(out, "Fine           : Rs %.2f\n", record->fineAmount);
    fprintf(out, "Payment status : %s\n", paymentStatusToString(record->paymentStatus));
}

void printSummaryReport(FILE *out, const ReportSummary_t *summary)
{
    int type;

    if (out == NULL || summary == NULL)
    {
        return;
    }
    fprintf(out, "Total records     : %zu\n", summary->totalRecords);
    fprintf(out, "Unpaid records    : %zu\n", summary->unpaidRecords);
    fprintf(out, "Total fines       : Rs %.2f\n", summary->totalFines);
    fprintf(out, "Collected fines   : Rs %.2f\n", summary->collectedFines);
    fprintf(out, "Outstanding fines : Rs %.2f\n", summary->outstandingFines);
    fprintf(out, "Violations by type:\n");
    for (type = 0; type < (int)VIOLATION_TYPE_COUNT; ++type)
    {
        fprintf(out, "  %-16s: %zu\n", violationTypeToString((ViolationType_t)type),
                summary->typeCounts[type]);
    }
}

Status_t writeSummaryReport(const RecordStore_t *store, const char *path, const char *generatedDate)
{
    ReportSummary_t summary;
    FILE *file;
    Status_t status;
    int failed;

    if (store == NULL || path == NULL || generatedDate == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    status = computeSummary(store, &summary);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    file = fopen(path, "w");
    if (file == NULL)
    {
        return STATUS_FILE_OPEN_FAILED;
    }
    fprintf(file, "Traffic Violation Summary Report (%s)\n", generatedDate);
    fprintf(file, "----------------------------------------\n");
    printSummaryReport(file, &summary);
    failed = ferror(file);
    if (fclose(file) != 0)
    {
        failed = 1;
    }
    return failed ? STATUS_FILE_IO_FAILED : STATUS_SUCCESS;
}
