/**
 * @file logger.c
 * @brief Logging functions implementation
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#include "logger.h"
#include "../utils/time_utils.h"

/**
 * @brief Ensures all necessary data directories exist.
 */
void ensure_BASE_DATA_DIRs(void)
{
  mkdir(BASE_DATA_DIR, 0755);        // Create main data directory
  mkdir(TRADES_LOG_DIR, 0755);   // Create trade log directory
  mkdir(METRICS_DIR, 0755);     // Create metrics directory
  mkdir(VWAP_DIR, 0755);        // Create VWAP directory
  mkdir(CORRELATION_DIR, 0755); // Create correlations directory
  mkdir(PERFORMANCE_LOGS_DIR, 0755); // Create performance directory
}

/**
 * @brief Helper function to open a log file for appending.
 * @param dir The directory path.
 * @param name The base file name.
 * @param ext The file extension.
 * @return File descriptor on success, -1 on error.
 */
int open_log_fd_append(const char *dir, const char *name, const char *ext)
{
  char path[256];

  snprintf(path, sizeof(path), "%s/%s.%s", dir, name, ext);

  return open(path, O_CREAT | O_WRONLY | O_APPEND | O_CLOEXEC, 0644);
}

/**
 * @brief Appends a raw trade message to its symbol-specific log file.
 * @param symbol_index Index of the symbol.
 * @param msg Pointer to raw trade message.
 */
void trade_log_append(int symbol_index, const raw_trade_message *msg)
{
  int fd = symbols[symbol_index].trade_log_fd;
  if (fd < 0)
  {
    fprintf(stderr, "ERROR: Trade log file descriptor not opened for symbol %s\n", 
            symbols[symbol_index].symbol);
    return;
  }

  /* CSV format: raw_json */
  char line[2048];
  int len = snprintf(line, sizeof(line), "%s\n", msg->raw_json);

  ssize_t result = write(fd, line, len);
  if (result < 0) {
    fprintf(stderr, "ERROR: Failed to write trade log for symbol %s: %s\n", 
            symbols[symbol_index].symbol, strerror(errno));
    return;
  }

  if (FSYNC_PER_WRITE)
  {
    if (fsync(fd) < 0) {
      fprintf(stderr, "WARNING: Failed to sync trade log for symbol %s: %s\n", 
              symbols[symbol_index].symbol, strerror(errno));
    }
  }
}

/**
 * @brief Logs system performance metrics (CPU, memory) to a CSV file.
 * @param timestamp_ms The timestamp of the measurement.
 * @param cpu_percent The CPU utilization percentage.
 * @param mem_mb The memory usage in megabytes.
 */
void log_system_metrics(int64_t timestamp_ms, double cpu_percent, double mem_mb)
{
  char path[256];
  snprintf(path, sizeof(path), "%s/system.csv", PERFORMANCE_LOGS_DIR);

  FILE *syslog = fopen(path, "a");
  if (!syslog)
  {
    fprintf(stderr, "ERROR: Failed to open system metrics log: %s\n", strerror(errno));
    return;
  }

  /* CSV format: timestamp_ms,cpu_percent,memory_mb */
  if (fprintf(syslog, "%" PRId64 ",%.2f,%.2f\n", timestamp_ms, cpu_percent, mem_mb) < 0) {
    fprintf(stderr, "WARNING: Failed to write system metrics\n");
  }

  fclose(syslog);
}

/**
 * @brief Timer precision and scheduling analysis.
 * @param scheduled_ms Scheduled timestamp in ms.
 * @param actual_ms Actual timestamp in ms.
 * @param drift_ns Drift in nanoseconds.
 */
void log_scheduler_metrics(int64_t scheduled_ms, int64_t actual_ms, int64_t drift_ns)
{
  char path[256];
  snprintf(path, sizeof(path), "%s/scheduler.csv", PERFORMANCE_LOGS_DIR);

  FILE *schedlog = fopen(path, "a");
  if (!schedlog)
  {
    fprintf(stderr, "ERROR: Failed to open scheduler metrics log: %s\n", strerror(errno));
    return;
  }

  double drift_ms = (double)drift_ns / NS_PER_MS;

  /* CSV format: scheduled_ms,actual_ms,drift_ms */
  if (fprintf(schedlog, "%" PRId64 ",%" PRId64 ",%.2f\n", scheduled_ms, actual_ms, drift_ms) < 0) {
    fprintf(stderr, "WARNING: Failed to write scheduler metrics\n");
  }

  fclose(schedlog);
}

/**
 * @brief Log latency metrics for a trade.
 * @param symbol_index Index of the symbol.
 * @param exchange_ts_ms Exchange timestamp.
 * @param recv_ts_ms Receive timestamp.
 * @param process_ts_ms Processing timestamp.
 */
void log_latency_metrics(int symbol_index, int64_t exchange_ts_ms, int64_t recv_ts_ms, int64_t process_ts_ms)
{
  if (latency_log_fd < 0)
  {
    fprintf(stderr, "ERROR: Latency log file descriptor not opened\n");
    return;
  }

  int64_t network_latency = recv_ts_ms - exchange_ts_ms;
  int64_t processing_latency = process_ts_ms - recv_ts_ms;
  int64_t total_latency = process_ts_ms - exchange_ts_ms;

  /* CSV format: symbol_index,exchange_ts,recv_ts,process_ts,network_lat,process_lat,total_lat */
  char line[256];
  int len = snprintf(line, sizeof(line),
                     "%d,%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 "\n",
                     symbol_index, exchange_ts_ms, recv_ts_ms, process_ts_ms,
                     network_latency, processing_latency, total_latency);

  ssize_t result = write(latency_log_fd, line, len);
  if (result < 0) {
    fprintf(stderr, "ERROR: Failed to write latency metrics: %s\n", strerror(errno));
    return;
  }

  if (FSYNC_PER_WRITE)
  {
    if (fsync(latency_log_fd) < 0) {
      fprintf(stderr, "WARNING: Failed to sync latency log: %s\n", strerror(errno));
    }
  }
}

/**
 * @brief Write moving statistics line to CSV.
 * @param idx Symbol index.
 * @param minute_ts_ms Minute timestamp.
 * @param vwap VWAP value.
 */
void vwap_log_append_csv(int idx, int64_t minute_ts_ms, double vwap)
{
  char path[256];
  snprintf(path, sizeof(path), "%s/%s.csv", VWAP_DIR, symbols[idx].symbol);
  FILE *fp = fopen(path, "a");

  if (!fp)
  {
    fprintf(stderr, "ERROR: Failed to open VWAP log file for %s: %s\n", 
            symbols[idx].symbol, strerror(errno));
    return;
  }

  char iso[64];
  format_minute_iso(minute_ts_ms, iso, sizeof(iso));

  if (fprintf(fp, "%s,%.12g\n", iso, vwap) < 0) {
    fprintf(stderr, "WARNING: Failed to write VWAP data for %s\n", symbols[idx].symbol);
  }

  fclose(fp);
}

/**
 * @brief Appends a correlation result to a CSV file.
 * @param symbol_idx The index of the base symbol.
 * @param minute_ts_ms The timestamp of the analysis.
 * @param other_symbol The symbol with the highest correlation.
 * @param corr The correlation coefficient.
 * @param lag_minute_ts_ms The timestamp of the correlated window.
 */
void correlation_log_append_csv(int symbol_idx, int64_t minute_ts_ms, const char *other_symbol, double corr, int64_t lag_minute_ts_ms)
{
  char path[256];
  snprintf(path, sizeof(path), "%s/%s.csv", CORRELATION_DIR, symbols[symbol_idx].symbol);
  FILE *fp = fopen(path, "a");

  if (!fp)
  {
    fprintf(stderr, "ERROR: Failed to open correlation log file for %s: %s\n", 
            symbols[symbol_idx].symbol, strerror(errno));
    return;
  }

  char iso[64], lagiso[64];
  format_minute_iso(minute_ts_ms, iso, sizeof(iso));

  if (lag_minute_ts_ms)
    format_minute_iso(lag_minute_ts_ms, lagiso, sizeof(lagiso));
  else
    strcpy(lagiso, "");

  /* CSV format: timestamp,correlated_with,correlation,lag_timestamp */
  if (fprintf(fp, "%s,%s,%.6g,%s\n", iso, other_symbol, corr, lagiso) < 0) {
    fprintf(stderr, "WARNING: Failed to write correlation data for %s\n", symbols[symbol_idx].symbol);
  }

  fclose(fp);
}

/**
 * @brief Initializes all log files and writes headers if they are new.
 */
void init_output_files(void)
{

  for (int i = 0; i < NUM_SYMBOLS; ++i)
  {
    /* open trade log files (kept open as file descriptors) */
    symbols[i].trade_log_fd = open_log_fd_append(TRADES_LOG_DIR, symbols[i].symbol, "jsonl");
    if (symbols[i].trade_log_fd < 0)
    {
      fprintf(stderr, "ERROR: Failed to open trade log file for %s: %s\n", 
              symbols[i].symbol, strerror(errno));
      symbols[i].trade_log_fd = -1;
    }

    /* initialize moving stats files with headers */
    int vwap_fd = open_log_fd_append(VWAP_DIR, symbols[i].symbol, "csv");
    if (vwap_fd >= 0)
    {
      struct stat st;
      if (fstat(vwap_fd, &st) == 0 && st.st_size == 0)
      {
        const char *moving_header = "timestamp_iso,vwap\n";
        ssize_t result = write(vwap_fd, moving_header, strlen(moving_header));
        if (result < 0) {
          fprintf(stderr, "WARNING: Failed to write VWAP header for %s\n", symbols[i].symbol);
        }
        if (FSYNC_PER_WRITE)
          fsync(vwap_fd);
      }
      close(vwap_fd);
    } else {
      fprintf(stderr, "ERROR: Failed to open VWAP log file for %s: %s\n", 
              symbols[i].symbol, strerror(errno));
    }

    /* initialize per-symbol correlation files */
    int corr_log_fd = open_log_fd_append(CORRELATION_DIR, symbols[i].symbol, "csv");
    if (corr_log_fd >= 0)
    {
      struct stat st;
      if (fstat(corr_log_fd, &st) == 0 && st.st_size == 0)
      {
        const char *corr_header = "timestamp_iso,correlated_with,correlation,lag_timestamp_iso\n";
        ssize_t result = write(corr_log_fd, corr_header, strlen(corr_header));
        if (result < 0) {
          fprintf(stderr, "WARNING: Failed to write correlation header for %s\n", symbols[i].symbol);
        }
        if (FSYNC_PER_WRITE)
          fsync(corr_log_fd);
      }
      close(corr_log_fd);
    }
  }

  /* initialize system resource log file */
  {
    int system_log_fd = open_log_fd_append(PERFORMANCE_LOGS_DIR, "system", "csv");
    if (system_log_fd >= 0)
    {
      struct stat st;
      if (fstat(system_log_fd, &st) == 0 && st.st_size == 0)
      {
        const char *header = "timestamp_ms,cpu_percent,memory_mb\n";
        ssize_t result = write(system_log_fd, header, strlen(header));
        if (result < 0) {
          fprintf(stderr, "WARNING: Failed to write system metrics header\n");
        }

        if (FSYNC_PER_WRITE)
          fsync(system_log_fd);
      }
      close(system_log_fd);
    }
  }

  /* initialize scheduler accuracy log file */
  {
    int scheduler_log_fd = open_log_fd_append(PERFORMANCE_LOGS_DIR, "scheduler", "csv");
    if (scheduler_log_fd >= 0)
    {
      struct stat st;
      if (fstat(scheduler_log_fd, &st) == 0 && st.st_size == 0)
      {
        const char *header = "scheduled_ms,actual_ms,drift_ms\n";
        ssize_t result = write(scheduler_log_fd, header, strlen(header));
        if (result < 0) {
          fprintf(stderr, "WARNING: Failed to write scheduler metrics header\n");
        }

        if (FSYNC_PER_WRITE)
          fsync(scheduler_log_fd);
      }
      close(scheduler_log_fd);
    }
  }

  /* open network latency log file (kept open as file descriptor) */
  latency_log_fd = open_log_fd_append(PERFORMANCE_LOGS_DIR, "latency", "csv");
  if (latency_log_fd >= 0)
  {
    struct stat st;
    if (fstat(latency_log_fd, &st) == 0 && st.st_size == 0)
    {
      const char *latency_header = "symbol_index,exchange_ts_ms,recv_ts_ms,process_ts_ms,"
                                   "network_latency_ms,processing_latency_ms,total_latency_ms\n";
      ssize_t result = write(latency_log_fd, latency_header, strlen(latency_header));
      if (result < 0) {
        fprintf(stderr, "WARNING: Failed to write latency metrics header\n");
      }

      if (FSYNC_PER_WRITE)
        fsync(latency_log_fd);
    }
  }
  else
  {
    perror("open network latency file");
  }
}