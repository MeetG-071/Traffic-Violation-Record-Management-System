#include "validation.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "date_utils.h"

#define DECIMAL_BASE 10

Status_t validateVehicleNumber(const char *vehicleNumber)
{
    size_t length;
    size_t i;

    if (vehicleNumber == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    length = strlen(vehicleNumber);
    if (length < VEHICLE_NUMBER_MIN_LEN || length > VEHICLE_NUMBER_MAX_LEN)
    {
        return STATUS_INVALID_VEHICLE_NUMBER;
    }
    for (i = 0; i < length; ++i)
    {
        unsigned char ch = (unsigned char)vehicleNumber[i];
        if (!(isupper(ch) || isdigit(ch)))
        {
            return STATUS_INVALID_VEHICLE_NUMBER;
        }
    }
    return STATUS_SUCCESS;
}

Status_t validateName(const char *name)
{
    size_t length;
    size_t i;
    int hasLetter = 0;

    if (name == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    length = strlen(name);
    if (length < 1 || length > DRIVER_NAME_MAX_LEN)
    {
        return STATUS_INVALID_NAME;
    }
    for (i = 0; i < length; ++i)
    {
        unsigned char ch = (unsigned char)name[i];
        if (isalpha(ch))
        {
            hasLetter = 1;
        }
        else if (!(ch == ' ' || ch == '.' || ch == '-' || ch == '\''))
        {
            return STATUS_INVALID_NAME;
        }
    }
    return hasLetter ? STATUS_SUCCESS : STATUS_INVALID_NAME;
}

Status_t validateLocation(const char *location)
{
    size_t length;
    size_t i;
    int hasVisible = 0;

    if (location == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    length = strlen(location);
    if (length < 1 || length > LOCATION_MAX_LEN)
    {
        return STATUS_INVALID_LOCATION;
    }
    for (i = 0; i < length; ++i)
    {
        unsigned char ch = (unsigned char)location[i];
        if (!isprint(ch) || ch == (unsigned char)FIELD_DELIMITER)
        {
            return STATUS_INVALID_LOCATION;
        }
        if (ch != ' ')
        {
            hasVisible = 1;
        }
    }
    return hasVisible ? STATUS_SUCCESS : STATUS_INVALID_LOCATION;
}

Status_t validateDate(const char *date)
{
    int year = 0;
    int month = 0;
    int day = 0;

    return parseDate(date, &year, &month, &day);
}

Status_t validateSpeedPair(int speed, int limit)
{
    if (limit < MIN_SPEED_KMH || limit > MAX_SPEED_KMH ||
        speed < MIN_SPEED_KMH || speed > MAX_SPEED_KMH ||
        speed <= limit)
    {
        return STATUS_INVALID_SPEED;
    }
    return STATUS_SUCCESS;
}

Status_t validateViolationType(int type)
{
    if (type < 0 || type >= (int)VIOLATION_TYPE_COUNT)
    {
        return STATUS_INVALID_TYPE;
    }
    return STATUS_SUCCESS;
}

Status_t validateFineAmount(double fine)
{
    /* Written so that NaN also fails. */
    if (!(fine > 0.0 && fine <= MAX_FINE_AMOUNT))
    {
        return STATUS_INVALID_FINE;
    }
    return STATUS_SUCCESS;
}

Status_t validateRecordFields(const ViolationRecord_t *record)
{
    Status_t status;

    if (record == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    status = validateVehicleNumber(record->vehicleNumber);
    if (status != STATUS_SUCCESS) { return status; }
    status = validateName(record->driverName);
    if (status != STATUS_SUCCESS) { return status; }
    status = validateLocation(record->location);
    if (status != STATUS_SUCCESS) { return status; }
    status = validateDate(record->date);
    if (status != STATUS_SUCCESS) { return status; }
    status = validateViolationType((int)record->type);
    if (status != STATUS_SUCCESS) { return status; }

    if (record->type == VIOLATION_SPEEDING)
    {
        return validateSpeedPair(record->recordedSpeed, record->speedLimit);
    }
    if (record->recordedSpeed != 0 || record->speedLimit != 0)
    {
        return STATUS_INVALID_SPEED;
    }
    return STATUS_SUCCESS;
}

Status_t parseInteger(const char *text, int min, int max, int *value)
{
    char *end = NULL;
    long parsed;

    if (text == NULL || value == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    if (text[0] == '\0' || isspace((unsigned char)text[0]))
    {
        return STATUS_INVALID_INPUT;
    }
    errno = 0;
    parsed = strtol(text, &end, DECIMAL_BASE);
    if (errno != 0 || end == text || *end != '\0')
    {
        return STATUS_INVALID_INPUT;
    }
    if (parsed < (long)min || parsed > (long)max)
    {
        return STATUS_INVALID_INPUT;
    }
    *value = (int)parsed;
    return STATUS_SUCCESS;
}

Status_t parseDouble(const char *text, double *value)
{
    char *end = NULL;
    double parsed;

    if (text == NULL || value == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    if (text[0] == '\0' || isspace((unsigned char)text[0]))
    {
        return STATUS_INVALID_INPUT;
    }
    errno = 0;
    parsed = strtod(text, &end);
    if (errno != 0 || end == text || *end != '\0' || !isfinite(parsed))
    {
        return STATUS_INVALID_INPUT;
    }
    *value = parsed;
    return STATUS_SUCCESS;
}
