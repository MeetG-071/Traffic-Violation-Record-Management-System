/*
 * violation.h
 * Core data model (violation record and record store) and record operations.
 */
#ifndef VIOLATION_H
#define VIOLATION_H

#include <stddef.h>

#include "config.h"
#include "status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    VIOLATION_SPEEDING = 0,
    VIOLATION_RED_LIGHT,
    VIOLATION_NO_HELMET,
    VIOLATION_NO_SEATBELT,
    VIOLATION_ILLEGAL_PARKING,
    VIOLATION_TYPE_COUNT
} ViolationType_t;

typedef enum
{
    PAYMENT_UNPAID = 0,
    PAYMENT_PAID,
    PAYMENT_STATUS_COUNT
} PaymentStatus_t;

typedef struct
{
    int id;
    char vehicleNumber[VEHICLE_NUMBER_MAX_LEN + 1];
    char driverName[DRIVER_NAME_MAX_LEN + 1];
    char location[LOCATION_MAX_LEN + 1];
    char date[DATE_STRING_LEN + 1];         /* YYYY-MM-DD */
    ViolationType_t type;
    int recordedSpeed;                      /* km/h, 0 when not speeding */
    int speedLimit;                         /* km/h, 0 when not speeding */
    double fineAmount;                      /* Indian Rupees */
    PaymentStatus_t paymentStatus;
} ViolationRecord_t;

typedef struct
{
    ViolationRecord_t records[MAX_RECORDS];
    size_t count;
    int nextId;
} RecordStore_t;

/* Resets the store to empty. nextId starts at 1. Ignores a NULL store. */
void initRecordStore(RecordStore_t *store);

/*
 * Calculates the fine for a violation.
 * priorViolations: earlier violations of the same vehicle (repeat-offender rule).
 * Returns STATUS_NULL_POINTER, STATUS_INVALID_TYPE, STATUS_INVALID_SPEED or STATUS_SUCCESS.
 */
Status_t calculateFine(ViolationType_t type, int speed, int limit,
                       size_t priorViolations, double *fine);

/*
 * Validates the input fields, rejects duplicates, computes the fine and appends
 * the record (id, fine and payment status are assigned by this function).
 */
Status_t addRecord(RecordStore_t *store, const ViolationRecord_t *input, int *newId);

/* Finds the array index of the record with the given id. */
Status_t findRecordIndexById(const RecordStore_t *store, int id, size_t *index);

/* Removes a record; later records shift down. */
Status_t deleteRecord(RecordStore_t *store, int id);

/* Marks an unpaid record as paid. */
Status_t markRecordPaid(RecordStore_t *store, int id);

/* Number of records stored for the given vehicle (0 for NULL arguments). */
size_t countViolationsByVehicle(const RecordStore_t *store, const char *vehicleNumber);

/* Sets *overdue to 1 when an unpaid fine is more than PAYMENT_DUE_DAYS old on 'today'. */
Status_t isRecordOverdue(const ViolationRecord_t *record, const char *today, int *overdue);

const char *violationTypeToString(ViolationType_t type);
const char *paymentStatusToString(PaymentStatus_t status);

#ifdef __cplusplus
}
#endif

#endif /* VIOLATION_H */
