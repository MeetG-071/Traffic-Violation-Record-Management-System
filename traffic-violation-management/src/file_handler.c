#include "file_handler.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "validation.h"

enum FileField
{
    FIELD_ID = 0,
    FIELD_VEHICLE,
    FIELD_DRIVER,
    FIELD_LOCATION,
    FIELD_DATE,
    FIELD_TYPE,
    FIELD_SPEED,
    FIELD_LIMIT,
    FIELD_FINE,
    FIELD_PAYMENT
};

/*
 * Splits 'line' in place at every delimiter. Returns the number of fields,
 * or maxFields + 1 when the line has too many.
 */
static size_t splitFields(char *line, char *fields[], size_t maxFields)
{
    size_t count = 0;
    char *cursor = line;

    fields[count++] = cursor;
    while (*cursor != '\0')
    {
        if (*cursor == FIELD_DELIMITER)
        {
            if (count == maxFields)
            {
                return maxFields + 1;
            }
            *cursor = '\0';
            fields[count++] = cursor + 1;
        }
        ++cursor;
    }
    return count;
}

static Status_t parseRecordLine(char *line, ViolationRecord_t *record)
{
    char *fields[FILE_FIELD_COUNT];
    int typeValue = 0;
    int paymentValue = 0;
    Status_t status;

    if (splitFields(line, fields, FILE_FIELD_COUNT) != FILE_FIELD_COUNT)
    {
        return STATUS_FILE_INVALID_DATA;
    }

    memset(record, 0, sizeof(*record));

    status = parseInteger(fields[FIELD_ID], 1, INT_MAX, &record->id);
    if (status != STATUS_SUCCESS) { return status; }
    status = validateVehicleNumber(fields[FIELD_VEHICLE]);
    if (status != STATUS_SUCCESS) { return status; }
    status = validateName(fields[FIELD_DRIVER]);
    if (status != STATUS_SUCCESS) { return status; }
    status = validateLocation(fields[FIELD_LOCATION]);
    if (status != STATUS_SUCCESS) { return status; }
    status = validateDate(fields[FIELD_DATE]);
    if (status != STATUS_SUCCESS) { return status; }

    snprintf(record->vehicleNumber, sizeof(record->vehicleNumber), "%s", fields[FIELD_VEHICLE]);
    snprintf(record->driverName, sizeof(record->driverName), "%s", fields[FIELD_DRIVER]);
    snprintf(record->location, sizeof(record->location), "%s", fields[FIELD_LOCATION]);
    snprintf(record->date, sizeof(record->date), "%s", fields[FIELD_DATE]);

    status = parseInteger(fields[FIELD_TYPE], 0, (int)VIOLATION_TYPE_COUNT - 1, &typeValue);
    if (status != STATUS_SUCCESS) { return status; }
    record->type = (ViolationType_t)typeValue;
    status = parseInteger(fields[FIELD_SPEED], 0, MAX_SPEED_KMH, &record->recordedSpeed);
    if (status != STATUS_SUCCESS) { return status; }
    status = parseInteger(fields[FIELD_LIMIT], 0, MAX_SPEED_KMH, &record->speedLimit);
    if (status != STATUS_SUCCESS) { return status; }
    status = parseDouble(fields[FIELD_FINE], &record->fineAmount);
    if (status != STATUS_SUCCESS) { return status; }
    status = validateFineAmount(record->fineAmount);
    if (status != STATUS_SUCCESS) { return status; }
    status = parseInteger(fields[FIELD_PAYMENT], 0, (int)PAYMENT_STATUS_COUNT - 1, &paymentValue);
    if (status != STATUS_SUCCESS) { return status; }
    record->paymentStatus = (PaymentStatus_t)paymentValue;

    return validateRecordFields(record);
}

static int idAlreadyStored(const RecordStore_t *store, int id)
{
    size_t i;

    for (i = 0; i < store->count; ++i)
    {
        if (store->records[i].id == id)
        {
            return 1;
        }
    }
    return 0;
}

Status_t loadRecordsFromFile(RecordStore_t *store, const char *path)
{
    FILE *file;
    char line[LINE_BUFFER_SIZE];
    ViolationRecord_t record;
    Status_t result = STATUS_SUCCESS;

    if (store == NULL || path == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    initRecordStore(store);

    file = fopen(path, "r");
    if (file == NULL)
    {
        return (errno == ENOENT) ? STATUS_FILE_NOT_FOUND : STATUS_FILE_OPEN_FAILED;
    }

    while (result == STATUS_SUCCESS && fgets(line, (int)sizeof(line), file) != NULL)
    {
        size_t length = strlen(line);

        if (length > 0 && line[length - 1] == '\n')
        {
            line[--length] = '\0';
        }
        else if (!feof(file))
        {
            result = STATUS_FILE_INVALID_DATA;     /* line longer than the buffer */
            break;
        }
        if (length > 0 && line[length - 1] == '\r')
        {
            line[--length] = '\0';
        }
        if (length == 0 || line[0] == COMMENT_PREFIX)
        {
            continue;
        }

        if (parseRecordLine(line, &record) != STATUS_SUCCESS ||
            store->count >= MAX_RECORDS ||
            idAlreadyStored(store, record.id))
        {
            result = STATUS_FILE_INVALID_DATA;
            break;
        }
        store->records[store->count++] = record;
        if (record.id >= store->nextId)
        {
            store->nextId = record.id + 1;
        }
    }

    if (result == STATUS_SUCCESS && ferror(file))
    {
        result = STATUS_FILE_IO_FAILED;
    }
    if (fclose(file) != 0 && result == STATUS_SUCCESS)
    {
        result = STATUS_FILE_IO_FAILED;
    }
    if (result != STATUS_SUCCESS)
    {
        initRecordStore(store);
    }
    return result;
}

Status_t saveRecordsToFile(const RecordStore_t *store, const char *path)
{
    FILE *file;
    size_t i;
    int failed = 0;

    if (store == NULL || path == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    file = fopen(path, "w");
    if (file == NULL)
    {
        return STATUS_FILE_OPEN_FAILED;
    }

    if (fprintf(file, "# id|vehicle|driver|location|date|type|speed|limit|fine|payment\n") < 0)
    {
        failed = 1;
    }
    for (i = 0; i < store->count && !failed; ++i)
    {
        const ViolationRecord_t *r = &store->records[i];
        if (fprintf(file, "%d|%s|%s|%s|%s|%d|%d|%d|%.2f|%d\n",
                    r->id, r->vehicleNumber, r->driverName, r->location, r->date,
                    (int)r->type, r->recordedSpeed, r->speedLimit, r->fineAmount,
                    (int)r->paymentStatus) < 0)
        {
            failed = 1;
        }
    }
    if (fclose(file) != 0)
    {
        failed = 1;
    }
    return failed ? STATUS_FILE_IO_FAILED : STATUS_SUCCESS;
}
