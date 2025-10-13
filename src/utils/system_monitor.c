/**
 * @file system_monitor.c
 * @brief System monitoring utility functions implementation
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#include "system_monitor.h"

/**
 * @brief Get CPU usage percentage since last call.
 * @details
 * Uses CLOCK_PROCESS_CPUTIME_ID for CPU time and CLOCK_MONOTONIC for wall time.
 * @param last_wall_time Pointer to last wall time (updated).
 * @param last_cpu_time Pointer to last CPU time (updated).
 * @return CPU usage percentage (0.0 on error or first call).
 */
double cpu_usage_percent_since(double *last_wall_time, double *last_cpu_time)
{
  struct timespec cpu_ts, wall_ts;

  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_ts) != 0 ||
      clock_gettime(CLOCK_MONOTONIC, &wall_ts) != 0)
  {
    perror("clock_gettime");
    return 0.0;
  }

  double cpu_time = cpu_ts.tv_sec + cpu_ts.tv_nsec / 1e9;    // Convert to seconds
  double wall_time = wall_ts.tv_sec + wall_ts.tv_nsec / 1e9; // Convert to seconds
  double usage = 0.0;

  if (*last_cpu_time != 0.0)
  { // Not the first call
    double diff_cpu = cpu_time - *last_cpu_time;
    double diff_wall = wall_time - *last_wall_time;

    if (diff_wall > 0)
    {
      usage = (diff_cpu / diff_wall) * 100.0;
    }
  }

  *last_cpu_time = cpu_time;
  *last_wall_time = wall_time;

  return usage;
}

/* Backward-compat wrapper: original name retained for callers */
double get_cpu_usage(double *last_wall_time, double *last_cpu_time)
{
  return cpu_usage_percent_since(last_wall_time, last_cpu_time);
}

/**
 * @brief Read current memory usage (VmRSS in kB) from /proc/self/status.
 * @return Memory usage in MB, or 0.0 on error.
 */
double memory_usage_mb(void)
{
  FILE *fp = fopen("/proc/self/status", "r");
  if (!fp)
  {
    perror("fopen /proc/self/status");
    return 0.0;
  }

  char line[256];
  long mem_kb = 0;

  while (fgets(line, sizeof(line), fp))
  {
    if (sscanf(line, "VmRSS:%ld kB", &mem_kb) == 1)
    {
      break;
    }
  }

  fclose(fp);
  return (double)mem_kb / 1024.0;
}

/* Backward-compat wrapper: original name retained for callers */
double get_memory_mb(void) 
{ 
  return memory_usage_mb(); 
}