/*
 * date_utils.h
 * Calendar helpers for YYYY-MM-DD date strings.
 */
#ifndef DATE_UTILS_H
#define DATE_UTILS_H

#include <stddef.h>

#include "status.h"

#ifdef __cplusplus
extern "C" {
#endif

int isLeapYear(int year);

/* Days in the month, or 0 for an invalid month. */
int daysInMonth(int year, int month);

/* Parses and fully validates a YYYY-MM-DD string. Returns STATUS_INVALID_DATE on failure. */
Status_t parseDate(const char *date, int *year, int *month, int *day);

/* Sets *diff to (to - from) in days; negative when 'to' is earlier. */
Status_t daysBetweenDates(const char *from, const char *to, long *diff);

/* Writes today's local date as YYYY-MM-DD. bufferSize must be >= DATE_STRING_LEN + 1. */
Status_t getTodayDate(char *buffer, size_t bufferSize);

#ifdef __cplusplus
}
#endif

#endif /* DATE_UTILS_H */
