/* Shared helpers for the unit tests. */
#ifndef TEST_HELPERS_HPP
#define TEST_HELPERS_HPP

#include <cstdio>
#include <cstring>

#include "violation.h"

/* Builds a valid record. Speeding records get 80 km/h in a 60 km/h zone. */
inline ViolationRecord_t makeRecord(const char *vehicle, const char *date, ViolationType_t type)
{
    ViolationRecord_t record;
    std::memset(&record, 0, sizeof(record));
    std::snprintf(record.vehicleNumber, sizeof(record.vehicleNumber), "%s", vehicle);
    std::snprintf(record.driverName, sizeof(record.driverName), "%s", "Rahul Patil");
    std::snprintf(record.location, sizeof(record.location), "%s", "FC Road Pune");
    std::snprintf(record.date, sizeof(record.date), "%s", date);
    record.type = type;
    if (type == VIOLATION_SPEEDING)
    {
        record.recordedSpeed = 80;
        record.speedLimit = 60;
    }
    return record;
}

#endif /* TEST_HELPERS_HPP */
