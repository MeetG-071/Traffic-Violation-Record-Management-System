/*
 * status.h
 * Common status codes returned by every module.
 */
#ifndef STATUS_H
#define STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    STATUS_SUCCESS = 0,
    STATUS_NULL_POINTER,
    STATUS_INVALID_ARGUMENT,
    STATUS_INVALID_INPUT,
    STATUS_INVALID_VEHICLE_NUMBER,
    STATUS_INVALID_NAME,
    STATUS_INVALID_LOCATION,
    STATUS_INVALID_DATE,
    STATUS_INVALID_SPEED,
    STATUS_INVALID_TYPE,
    STATUS_INVALID_FINE,
    STATUS_STORAGE_FULL,
    STATUS_NOT_FOUND,
    STATUS_EMPTY_STORE,
    STATUS_DUPLICATE,
    STATUS_ALREADY_PAID,
    STATUS_FILE_NOT_FOUND,
    STATUS_FILE_OPEN_FAILED,
    STATUS_FILE_IO_FAILED,
    STATUS_FILE_INVALID_DATA,
    STATUS_MEMORY_FAILURE,
    STATUS_END_OF_INPUT
} Status_t;

/* Returns a human-readable message for a status code (never NULL). */
const char *statusToString(Status_t status);

#ifdef __cplusplus
}
#endif

#endif /* STATUS_H */
