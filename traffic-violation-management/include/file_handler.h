/*
 * file_handler.h
 * Text-file persistence for the record store.
 *
 * Record format (one per line, '|' separated):
 *   id|vehicle|driver|location|date|type|speed|limit|fine|payment
 * Blank lines and lines starting with '#' are ignored when loading.
 */
#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "status.h"
#include "violation.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Loads records from 'path' into 'store' (the store is reset first).
 * STATUS_FILE_NOT_FOUND         - file does not exist (store left empty)
 * STATUS_FILE_OPEN_FAILED       - file exists but cannot be opened
 * STATUS_FILE_INVALID_DATA      - malformed line, duplicate id or capacity exceeded (store left empty)
 * STATUS_FILE_IO_FAILED         - read error
 */
Status_t loadRecordsFromFile(RecordStore_t *store, const char *path);

/* Writes every record to 'path', replacing its contents. */
Status_t saveRecordsToFile(const RecordStore_t *store, const char *path);

#ifdef __cplusplus
}
#endif

#endif /* FILE_HANDLER_H */
