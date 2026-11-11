/*
 * validation.h
 * Input validation and safe text-to-number parsing.
 */
#ifndef VALIDATION_H
#define VALIDATION_H

#include "status.h"
#include "violation.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 6-10 characters, uppercase letters and digits only. */
Status_t validateVehicleNumber(const char *vehicleNumber);

/* 1-50 characters, letters, spaces, '.', '-' and '\''; at least one letter. */
Status_t validateName(const char *name);

/* 1-60 printable characters, not blank, must not contain the file delimiter. */
Status_t validateLocation(const char *location);

/* Strict YYYY-MM-DD, real calendar date, year within MIN_YEAR..MAX_YEAR. */
Status_t validateDate(const char *date);

/* Both speeds within range and speed strictly above limit. */
Status_t validateSpeedPair(int speed, int limit);

Status_t validateViolationType(int type);

/* 0 < fine <= MAX_FINE_AMOUNT. */
Status_t validateFineAmount(double fine);

/* Validates every user-supplied field of a record (not id, fine or payment). */
Status_t validateRecordFields(const ViolationRecord_t *record);

/* Strict integer parsing: whole text must be a number inside [min, max]. */
Status_t parseInteger(const char *text, int min, int max, int *value);

/* Strict floating-point parsing: whole text must be a finite number. */
Status_t parseDouble(const char *text, double *value);

#ifdef __cplusplus
}
#endif

#endif /* VALIDATION_H */
