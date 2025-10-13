/**
 * @file time_utils.c
 * @brief Time utility functions implementation
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#include "time_utils.h"

/**
 * @brief Get current real-world time in milliseconds since the epoch.
 * @return Current time in ms.
 */
int64_t now_ms(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

/**
 * @brief Get current monotonic time in nanoseconds for precise interval measurements.
 * @return Monotonic time in ns.
 */
int64_t now_monotonic_ns(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (int64_t)ts.tv_sec * NS_PER_SEC + ts.tv_nsec;
}

/**
 * @brief Format a millisecond timestamp to an ISO 8601 string (YYYY-MM-DDTHH:MM:00Z).
 * @param ms Timestamp in milliseconds.
 * @param buf Output buffer.
 * @param bufsz Size of buffer.
 */
void format_minute_iso(int64_t ms, char *buf, size_t bufsz)
{
  time_t sec = ms / 1000;
  struct tm tm;
  localtime_r(&sec, &tm);
  strftime(buf, bufsz, "%Y-%m-%dT%H:%M:00%z", &tm);
}