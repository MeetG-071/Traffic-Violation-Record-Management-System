#include "violation.h"

#include <string.h>

#include "date_utils.h"
#include "validation.h"

static int isDuplicateRecord(const RecordStore_t *store, const ViolationRecord_t *candidate)
{
    size_t i;

    for (i = 0; i < store->count; ++i)
    {
        const ViolationRecord_t *existing = &store->records[i];
        if (existing->type == candidate->type &&
            strcmp(existing->vehicleNumber, candidate->vehicleNumber) == 0 &&
            strcmp(existing->date, candidate->date) == 0 &&
            strcmp(existing->location, candidate->location) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void initRecordStore(RecordStore_t *store)
{
    if (store == NULL)
    {
        return;
    }
    memset(store, 0, sizeof(*store));
    store->nextId = 1;
}

Status_t calculateFine(ViolationType_t type, int speed, int limit,
                       size_t priorViolations, double *fine)
{
    double amount = 0.0;
    Status_t status;

    if (fine == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    status = validateViolationType((int)type);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    switch (type)
    {
        case VIOLATION_SPEEDING:
            status = validateSpeedPair(speed, limit);
            if (status != STATUS_SUCCESS)
            {
                return status;
            }
            amount = FINE_SPEEDING_BASE + ((double)(speed - limit) * FINE_SPEEDING_PER_KMH);
            break;
        case VIOLATION_RED_LIGHT:      amount = FINE_RED_LIGHT;      break;
        case VIOLATION_NO_HELMET:      amount = FINE_NO_HELMET;      break;
        case VIOLATION_NO_SEATBELT:    amount = FINE_NO_SEATBELT;    break;
        case VIOLATION_ILLEGAL_PARKING: amount = FINE_ILLEGAL_PARKING; break;
        default:                       return STATUS_INVALID_TYPE;
    }

    if (priorViolations >= REPEAT_OFFENDER_THRESHOLD)
    {
        amount *= REPEAT_OFFENDER_MULTIPLIER;
    }
    if (amount > MAX_FINE_AMOUNT)
    {
        amount = MAX_FINE_AMOUNT;
    }
    *fine = amount;
    return STATUS_SUCCESS;
}

size_t countViolationsByVehicle(const RecordStore_t *store, const char *vehicleNumber)
{
    size_t matches = 0;
    size_t i;

    if (store == NULL || vehicleNumber == NULL)
    {
        return 0;
    }
    for (i = 0; i < store->count; ++i)
    {
        if (strcmp(store->records[i].vehicleNumber, vehicleNumber) == 0)
        {
            ++matches;
        }
    }
    return matches;
}

Status_t addRecord(RecordStore_t *store, const ViolationRecord_t *input, int *newId)
{
    ViolationRecord_t *slot;
    double fine = 0.0;
    Status_t status;

    if (store == NULL || input == NULL || newId == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    status = validateRecordFields(input);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    if (store->count >= MAX_RECORDS)
    {
        return STATUS_STORAGE_FULL;
    }
    if (isDuplicateRecord(store, input))
    {
        return STATUS_DUPLICATE;
    }
    status = calculateFine(input->type, input->recordedSpeed, input->speedLimit,
                           countViolationsByVehicle(store, input->vehicleNumber), &fine);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    slot = &store->records[store->count];
    *slot = *input;
    slot->id = store->nextId;
    slot->fineAmount = fine;
    slot->paymentStatus = PAYMENT_UNPAID;

    *newId = slot->id;
    store->count++;
    store->nextId++;
    return STATUS_SUCCESS;
}

Status_t findRecordIndexById(const RecordStore_t *store, int id, size_t *index)
{
    size_t i;

    if (store == NULL || index == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    if (store->count == 0)
    {
        return STATUS_EMPTY_STORE;
    }
    for (i = 0; i < store->count; ++i)
    {
        if (store->records[i].id == id)
        {
            *index = i;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}

Status_t deleteRecord(RecordStore_t *store, int id)
{
    size_t index = 0;
    Status_t status = findRecordIndexById(store, id, &index);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    memmove(&store->records[index], &store->records[index + 1],
            (store->count - index - 1) * sizeof(store->records[0]));
    store->count--;
    memset(&store->records[store->count], 0, sizeof(store->records[0]));
    return STATUS_SUCCESS;
}

Status_t markRecordPaid(RecordStore_t *store, int id)
{
    size_t index = 0;
    Status_t status = findRecordIndexById(store, id, &index);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    if (store->records[index].paymentStatus == PAYMENT_PAID)
    {
        return STATUS_ALREADY_PAID;
    }
    store->records[index].paymentStatus = PAYMENT_PAID;
    return STATUS_SUCCESS;
}

Status_t isRecordOverdue(const ViolationRecord_t *record, const char *today, int *overdue)
{
    long elapsedDays = 0;
    Status_t status;

    if (record == NULL || today == NULL || overdue == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    *overdue = 0;
    if (record->paymentStatus == PAYMENT_PAID)
    {
        return STATUS_SUCCESS;
    }
    status = daysBetweenDates(record->date, today, &elapsedDays);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    *overdue = (elapsedDays > PAYMENT_DUE_DAYS) ? 1 : 0;
    return STATUS_SUCCESS;
}

const char *violationTypeToString(ViolationType_t type)
{
    switch (type)
    {
        case VIOLATION_SPEEDING:        return "Speeding";
        case VIOLATION_RED_LIGHT:       return "Red light";
        case VIOLATION_NO_HELMET:       return "No helmet";
        case VIOLATION_NO_SEATBELT:     return "No seatbelt";
        case VIOLATION_ILLEGAL_PARKING: return "Illegal parking";
        default:                        return "Unknown";
    }
}

const char *paymentStatusToString(PaymentStatus_t status)
{
    switch (status)
    {
        case PAYMENT_UNPAID: return "Unpaid";
        case PAYMENT_PAID:   return "Paid";
        default:             return "Unknown";
    }
}
