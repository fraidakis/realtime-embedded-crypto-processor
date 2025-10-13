/**
 * @file logger.h
 * @brief Logging functions declarations
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#ifndef LOGGER_H
#define LOGGER_H

#include "../../include/common.h"

/**
 * @brief Ensures all necessary data directories exist.
 */
void ensure_BASE_DATA_DIRs(void);

/**
 * @brief Helper function to open a log file for appending.
 * @param dir The directory path.
 * @param name The base file name.
 * @param ext The file extension.
 * @return File descriptor on success, -1 on error.
 */
int open_log_fd_append(const char *dir, const char *name, const char *ext);

/**
 * @brief Appends a raw trade message to its symbol-specific log file.
 * @param symbol_index Index of the symbol.
 * @param msg Pointer to raw trade message.
 */
void trade_log_append(int symbol_index, const raw_trade_message *msg);

/**
 * @brief Logs system performance metrics (CPU, memory) to a CSV file.
 * @param timestamp_ms The timestamp of the measurement.
 * @param cpu_percent The CPU utilization percentage.
 * @param mem_mb The memory usage in megabytes.
 */
void log_system_metrics(int64_t timestamp_ms, double cpu_percent, double mem_mb);

/**
 * @brief Timer precision and scheduling analysis.
 * @param scheduled_ms Scheduled timestamp in ms.
 * @param actual_ms Actual timestamp in ms.
 * @param drift_ns Drift in nanoseconds.
 */
void log_scheduler_metrics(int64_t scheduled_ms, int64_t actual_ms, int64_t drift_ns);

/**
 * @brief Log latency metrics for a trade.
 * @param symbol_index Index of the symbol.
 * @param exchange_ts_ms Exchange timestamp.
 * @param recv_ts_ms Receive timestamp.
 * @param process_ts_ms Processing timestamp.
 */
void log_latency_metrics(int symbol_index, int64_t exchange_ts_ms, int64_t recv_ts_ms, int64_t process_ts_ms);

/**
 * @brief Write moving statistics line to CSV.
 * @param idx Symbol index.
 * @param minute_ts_ms Minute timestamp.
 * @param vwap VWAP value.
 */
void vwap_log_append_csv(int idx, int64_t minute_ts_ms, double vwap);

/**
 * @brief Appends a correlation result to a CSV file.
 * @param symbol_idx The index of the base symbol.
 * @param minute_ts_ms The timestamp of the analysis.
 * @param other_symbol The symbol with the highest correlation.
 * @param corr The correlation coefficient.
 * @param lag_minute_ts_ms The timestamp of the correlated window.
 */
void correlation_log_append_csv(int symbol_idx, int64_t minute_ts_ms, const char *other_symbol, double corr, int64_t lag_minute_ts_ms);

/**
 * @brief Initializes all log files and writes headers if they are new.
 */
void init_output_files(void);

#endif /* LOGGER_H */