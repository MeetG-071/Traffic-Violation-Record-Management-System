/*
 * report.h
 * Statistics, filtering, sorting and printing of violation records.
 */
#ifndef REPORT_H
#define REPORT_H

#include <stddef.h>
#include <stdio.h>

#include "status.h"
#include "violation.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    size_t totalRecords;
    size_t unpaidRecords;
    double totalFines;
    double collectedFines;
    double outstandingFines;
    size_t typeCounts[VIOLATION_TYPE_COUNT];
} ReportSummary_t;

/* Computes totals. Returns STATUS_EMPTY_STORE (summary zeroed) when there are no records. */
Status_t computeSummary(const RecordStore_t *store, ReportSummary_t *summary);

/* Sorts in place: highest fine first, ties by lower id. */
Status_t sortRecordsByFineDescending(ViolationRecord_t *records, size_t count);

/*
 * The functions below allocate *out with malloc; the caller must free() it.
 * STATUS_EMPTY_STORE / STATUS_NOT_FOUND mean nothing was allocated.
 */
Status_t buildSortedByFine(const RecordStore_t *store, ViolationRecord_t **out, size_t *count);
Status_t collectVehicleRecords(const RecordStore_t *store, const char *vehicleNumber,
                               ViolationRecord_t **out, size_t *count);
Status_t collectOverdueRecords(const RecordStore_t *store, const char *today,
                               ViolationRecord_t **out, size_t *count);

/* Number of overdue records on 'today'. */
Status_t countOverdueRecords(const RecordStore_t *store, const char *today, size_t *count);

void printRecordTable(FILE *out, const ViolationRecord_t *records, size_t count);
void printRecordDetails(FILE *out, const ViolationRecord_t *record);
void printSummaryReport(FILE *out, const ReportSummary_t *summary);

/* Computes the summary and writes it to a text file. */
Status_t writeSummaryReport(const RecordStore_t *store, const char *path, const char *generatedDate);

#ifdef __cplusplus
}
#endif

#endif /* REPORT_H */
