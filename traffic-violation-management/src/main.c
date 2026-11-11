/*
 * main.c
 * Application entry point: loads data, runs the menu loop and saves changes.
 * All real work is done by the other modules.
 */
#include <stdio.h>

#include "config.h"
#include "file_handler.h"
#include "ui.h"
#include "violation.h"

static void persist(const RecordStore_t *store, const char *path)
{
    Status_t status = saveRecordsToFile(store, path);

    if (status != STATUS_SUCCESS)
    {
        printf("Warning: could not save to %s: %s\n", path, statusToString(status));
    }
}

int main(int argc, char *argv[])
{
    const char *dataFile = (argc > 1) ? argv[1] : DEFAULT_DATA_FILE;
    RecordStore_t store;
    Status_t status;
    int running = 1;

    initRecordStore(&store);
    status = loadRecordsFromFile(&store, dataFile);
    if (status == STATUS_FILE_NOT_FOUND)
    {
        printf("No data file at %s. Starting with an empty database.\n", dataFile);
    }
    else if (status != STATUS_SUCCESS)
    {
        printf("Cannot start: %s (%s). Fix or remove the file and try again.\n",
               statusToString(status), dataFile);
        return 1;
    }
    else
    {
        printf("Loaded %zu record(s) from %s.\n", store.count, dataFile);
    }

    while (running)
    {
        int choice = 0;
        int modified = 0;

        displayMainMenu();
        status = readIntegerInRange("Enter choice: ", 0, 9, &choice);
        if (status == STATUS_END_OF_INPUT)
        {
            break;
        }
        if (status != STATUS_SUCCESS)
        {
            printf("Unsupported menu choice. Enter a number from 0 to 9.\n");
            continue;
        }

        switch (choice)
        {
            case 1: modified = handleAddRecord(&store);   break;
            case 2: handleListRecords(&store);            break;
            case 3: handleSearchById(&store);             break;
            case 4: handleVehicleHistory(&store);         break;
            case 5: modified = handleMarkPaid(&store);    break;
            case 6: modified = handleDeleteRecord(&store); break;
            case 7: handleOverdueReport(&store);          break;
            case 8: handleSortedByFine(&store);           break;
            case 9: handleSummaryReport(&store);          break;
            case 0: running = 0;                          break;
            default: break;
        }

        if (modified)
        {
            persist(&store, dataFile);
        }
    }

    printf("Goodbye.\n");
    return 0;
}
