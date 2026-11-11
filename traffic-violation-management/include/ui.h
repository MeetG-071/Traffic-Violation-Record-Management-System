/*
 * ui.h
 * Console user interface: safe input helpers, menu and menu-action handlers.
 * Handlers that may change data return 1 when the store was modified, otherwise 0.
 */
#ifndef UI_H
#define UI_H

#include <stddef.h>

#include "status.h"
#include "violation.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Reads one line (without newline). STATUS_END_OF_INPUT on EOF, STATUS_INVALID_INPUT if too long. */
Status_t readLine(const char *prompt, char *buffer, size_t size);

/* Reads one line and parses it as an integer in [min, max]. */
Status_t readIntegerInRange(const char *prompt, int min, int max, int *value);

void displayMainMenu(void);

int handleAddRecord(RecordStore_t *store);
void handleListRecords(const RecordStore_t *store);
void handleSearchById(const RecordStore_t *store);
void handleVehicleHistory(const RecordStore_t *store);
int handleMarkPaid(RecordStore_t *store);
int handleDeleteRecord(RecordStore_t *store);
void handleOverdueReport(const RecordStore_t *store);
void handleSortedByFine(const RecordStore_t *store);
void handleSummaryReport(const RecordStore_t *store);

#ifdef __cplusplus
}
#endif

#endif /* UI_H */
