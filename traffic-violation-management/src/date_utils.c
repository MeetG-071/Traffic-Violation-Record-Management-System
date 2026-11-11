#include "date_utils.h"

#include <ctype.h>
#include <time.h>

#include "config.h"

#define DATE_YEAR_DIGITS    4
#define DATE_FIELD_DIGITS   2
#define DATE_FIRST_DASH     4
#define DATE_SECOND_DASH    7
#define MONTHS_PER_YEAR     12

static int readDigits(const char *text, int count, int *value)
{
    int result = 0;
    int i;

    for (i = 0; i < count; ++i)
    {
        if (!isdigit((unsigned char)text[i]))
        {
            return 0;
        }
        result = (result * 10) + (text[i] - '0');
    }
    *value = result;
    return 1;
}

/* Days since 1970-01-01 for a valid civil date (proleptic Gregorian calendar). */
static long dateToDayNumber(int year, int month, int day)
{
    long adjustedYear = (month <= 2) ? (long)year - 1 : (long)year;
    long era = adjustedYear / 400;
    long yearOfEra = adjustedYear - (era * 400);
    long monthIndex = (month > 2) ? (long)month - 3 : (long)month + 9;
    long dayOfYear = ((153 * monthIndex) + 2) / 5 + (day - 1);
    long dayOfEra = (yearOfEra * 365) + (yearOfEra / 4) - (yearOfEra / 100) + dayOfYear;

    return (era * 146097) + dayOfEra - 719468;
}

int isLeapYear(int year)
{
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

int daysInMonth(int year, int month)
{
    static const int DAYS[MONTHS_PER_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month < 1 || month > MONTHS_PER_YEAR)
    {
        return 0;
    }
    if (month == 2 && isLeapYear(year))
    {
        return 29;
    }
    return DAYS[month - 1];
}

Status_t parseDate(const char *date, int *year, int *month, int *day)
{
    int y = 0;
    int m = 0;
    int d = 0;
    size_t i;

    if (date == NULL || year == NULL || month == NULL || day == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    for (i = 0; i < DATE_STRING_LEN; ++i)
    {
        if (date[i] == '\0')
        {
            return STATUS_INVALID_DATE;
        }
    }
    if (date[DATE_STRING_LEN] != '\0' ||
        date[DATE_FIRST_DASH] != '-' || date[DATE_SECOND_DASH] != '-')
    {
        return STATUS_INVALID_DATE;
    }
    if (!readDigits(date, DATE_YEAR_DIGITS, &y) ||
        !readDigits(date + DATE_FIRST_DASH + 1, DATE_FIELD_DIGITS, &m) ||
        !readDigits(date + DATE_SECOND_DASH + 1, DATE_FIELD_DIGITS, &d))
    {
        return STATUS_INVALID_DATE;
    }
    if (y < MIN_YEAR || y > MAX_YEAR || m < 1 || m > MONTHS_PER_YEAR ||
        d < 1 || d > daysInMonth(y, m))
    {
        return STATUS_INVALID_DATE;
    }
    *year = y;
    *month = m;
    *day = d;
    return STATUS_SUCCESS;
}

Status_t daysBetweenDates(const char *from, const char *to, long *diff)
{
    int y1 = 0, m1 = 0, d1 = 0;
    int y2 = 0, m2 = 0, d2 = 0;
    Status_t status;

    if (diff == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    status = parseDate(from, &y1, &m1, &d1);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    status = parseDate(to, &y2, &m2, &d2);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    *diff = dateToDayNumber(y2, m2, d2) - dateToDayNumber(y1, m1, d1);
    return STATUS_SUCCESS;
}

Status_t getTodayDate(char *buffer, size_t bufferSize)
{
    time_t now;
    struct tm *local;

    if (buffer == NULL)
    {
        return STATUS_NULL_POINTER;
    }
    if (bufferSize < (size_t)DATE_STRING_LEN + 1)
    {
        return STATUS_INVALID_ARGUMENT;
    }
    now = time(NULL);
    if (now == (time_t)-1)
    {
        return STATUS_INVALID_DATE;
    }
    local = localtime(&now);
    if (local == NULL || strftime(buffer, bufferSize, "%Y-%m-%d", local) == 0)
    {
        return STATUS_INVALID_DATE;
    }
    return STATUS_SUCCESS;
}
