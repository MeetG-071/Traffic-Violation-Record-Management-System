#include "status.h"

const char *statusToString(Status_t status)
{
    switch (status)
    {
        case STATUS_SUCCESS:              return "Success";
        case STATUS_NULL_POINTER:         return "Internal error: null pointer";
        case STATUS_INVALID_ARGUMENT:     return "Invalid argument";
        case STATUS_INVALID_INPUT:        return "Invalid input (empty, too long or not a valid number)";
        case STATUS_INVALID_VEHICLE_NUMBER:
            return "Invalid vehicle number (6-10 uppercase letters/digits)";
        case STATUS_INVALID_NAME:         return "Invalid name (1-50 letters, spaces, '.', '-' or apostrophe)";
        case STATUS_INVALID_LOCATION:     return "Invalid location (1-60 printable characters, no '|')";
        case STATUS_INVALID_DATE:         return "Invalid date (use a real date as YYYY-MM-DD, 2000-2100)";
        case STATUS_INVALID_SPEED:        return "Invalid speed (speed must exceed the limit, 1-250 km/h)";
        case STATUS_INVALID_TYPE:         return "Invalid violation type";
        case STATUS_INVALID_FINE:         return "Invalid fine amount";
        case STATUS_STORAGE_FULL:         return "Storage is full";
        case STATUS_NOT_FOUND:            return "Record not found";
        case STATUS_EMPTY_STORE:          return "There are no records";
        case STATUS_DUPLICATE:            return "Duplicate record (same vehicle, date, type and location)";
        case STATUS_ALREADY_PAID:         return "The fine is already paid";
        case STATUS_FILE_NOT_FOUND:       return "File not found";
        case STATUS_FILE_OPEN_FAILED:     return "File could not be opened";
        case STATUS_FILE_IO_FAILED:       return "File read/write failed";
        case STATUS_FILE_INVALID_DATA:    return "File contains invalid data";
        case STATUS_MEMORY_FAILURE:       return "Memory allocation failed";
        case STATUS_END_OF_INPUT:         return "End of input";
        default:                          return "Unknown status";
    }
}
