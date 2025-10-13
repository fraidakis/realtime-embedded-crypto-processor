/**
 * @file time_utils.h
 * @brief Time utility functions declarations
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include "../../include/common.h"

/**
 * @brief Get current real-world time in milliseconds since the epoch.
 * @return Current time in ms.
 */
int64_t now_ms(void);

/**
 * @brief Get current monotonic time in nanoseconds for precise interval measurements.
 * @return Monotonic time in ns.
 */
int64_t now_monotonic_ns(void);

/**
 * @brief Format a millisecond timestamp to an ISO 8601 string (YYYY-MM-DDTHH:MM:00Z).
 * @param ms Timestamp in milliseconds.
 * @param buf Output buffer.
 * @param bufsz Size of buffer.
 */
void format_minute_iso(int64_t ms, char *buf, size_t bufsz);

#endif /* TIME_UTILS_H */