/**
 * @file system_monitor.h
 * @brief System monitoring utility functions declarations
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include "../../include/common.h"

/**
 * @brief Get CPU usage percentage since last call.
 * @details
 * Uses CLOCK_PROCESS_CPUTIME_ID for CPU time and CLOCK_MONOTONIC for wall time.
 * @param last_wall_time Pointer to last wall time (updated).
 * @param last_cpu_time Pointer to last CPU time (updated).
 * @return CPU usage percentage (0.0 on error or first call).
 */
double cpu_usage_percent_since(double *last_wall_time, double *last_cpu_time);

/* Backward-compat wrapper: original name retained for callers */
double get_cpu_usage(double *last_wall_time, double *last_cpu_time);

/**
 * @brief Read current memory usage (VmRSS in kB) from /proc/self/status.
 * @return Memory usage in MB, or 0.0 on error.
 */
double memory_usage_mb(void);

/* Backward-compat wrapper: original name retained for callers */
double get_memory_mb(void);

#endif /* SYSTEM_MONITOR_H */