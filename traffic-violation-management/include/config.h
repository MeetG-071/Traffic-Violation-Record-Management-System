/*
 * config.h
 * Project-wide named constants: sizes, valid ranges, fine rules and file names.
 */
#ifndef CONFIG_H
#define CONFIG_H

/* Storage capacity */
#define MAX_RECORDS                 500

/* String-size limits (characters, excluding the terminating '\0') */
#define VEHICLE_NUMBER_MIN_LEN      6
#define VEHICLE_NUMBER_MAX_LEN      10
#define DRIVER_NAME_MAX_LEN         50
#define LOCATION_MAX_LEN            60
#define DATE_STRING_LEN             10      /* YYYY-MM-DD */

/* Valid ranges */
#define MIN_YEAR                    2000
#define MAX_YEAR                    2100
#define MIN_SPEED_KMH               1
#define MAX_SPEED_KMH               250
#define MAX_FINE_AMOUNT             100000.0

/* Fine rules (Indian Rupees) */
#define FINE_SPEEDING_BASE          1000.0
#define FINE_SPEEDING_PER_KMH       100.0
#define FINE_RED_LIGHT              1000.0
#define FINE_NO_HELMET              500.0
#define FINE_NO_SEATBELT            500.0
#define FINE_ILLEGAL_PARKING        300.0
#define REPEAT_OFFENDER_THRESHOLD   3       /* prior violations before penalty */
#define REPEAT_OFFENDER_MULTIPLIER  2.0

/* Payment rule */
#define PAYMENT_DUE_DAYS            30

/* File format */
#define FIELD_DELIMITER             '|'
#define FILE_FIELD_COUNT            10
#define LINE_BUFFER_SIZE            256
#define COMMENT_PREFIX              '#'

/* Console input */
#define INPUT_BUFFER_SIZE           128

/* Default file names (relative to the repository root) */
#define DEFAULT_DATA_FILE           "data/violations.txt"
#define SUMMARY_REPORT_FILE         "data/summary_report.txt"

#endif /* CONFIG_H */
