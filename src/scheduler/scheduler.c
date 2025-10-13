/**
 * @file scheduler.c
 * @brief Scheduler thread implementation
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#include "scheduler.h"
#include "../utils/time_utils.h"
#include "../utils/system_monitor.h"
#include "../logging/logger.h"

/**
 * @brief Coordinator thread that schedules the worker threads to run precisely every minute.
 * @param arg Thread argument (unused).
 * @return NULL.
 */
void *scheduler_thread_fn(void *arg)
{
  (void)arg;

  /* Performance monitoring variables */
  double cpu_last_time = 0.0;
  double cpu_last_usage = 0.0;

  /* EMA for computation duration (in nanoseconds) */
  double ema_duration_ns = 0.0;
  const double ema_alpha = 0.2;
  const double EMA_MAX_NS = 100.0 * NS_PER_MS;
  const int64_t PERIOD_NS = 60000LL * NS_PER_MS;

  /* Align to next minute boundary in nanoseconds */
  int64_t now_ns = now_monotonic_ns();
  int64_t scheduled_time_ns = ((now_ns / PERIOD_NS) + 1) * PERIOD_NS;

  while (!shutdown_requested)
  {
    now_ns = now_monotonic_ns();

    /* Advance scheduled_time_ns until it's in the future */
    while (scheduled_time_ns <= now_ns)
      scheduled_time_ns += PERIOD_NS;

    /* Compute target wake time */
    int64_t predicted_duration_ns = (int64_t)llround(ema_duration_ns);
    int64_t target_wakeup_ns = scheduled_time_ns - predicted_duration_ns;

    /* Ensure target_wakeup_ns is valid and in the future */
    if (target_wakeup_ns <= now_ns)
    {
      int64_t late_by_ns = now_ns - (scheduled_time_ns - predicted_duration_ns);
      fprintf(stderr, "WARNING: Missed schedule window (late by %.2f ms), executing immediately\n",
                late_by_ns / (double)NS_PER_MS);
      target_wakeup_ns = now_ns;
    }

    /* Convert to timespec (with validation) */
    struct timespec wake_ts;
    wake_ts.tv_sec = target_wakeup_ns / NS_PER_SEC;
    wake_ts.tv_nsec = target_wakeup_ns % NS_PER_SEC;

    /* Sleep until target time, handling interruptions */
    while (!shutdown_requested)
    {
      int ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake_ts, NULL);

      if (ret == 0)
      {
        /* Sleep completed successfully */
        break;
      }
      else if (ret == EINTR)
      {
        /* Interrupted by signal, retry the same absolute time */
        continue;
      }
      else
      {
        /* Unexpected error */
        fprintf(stderr, "ERROR: clock_nanosleep failed: %s\n", strerror(ret));
        break;
      }
    }

    if (shutdown_requested)
      break;

    /* Record current minute timestamp (aligned to minute boundary) */
    current_minute_ms = (now_ms() / MS_PER_MINUTE) * MS_PER_MINUTE;

    /* Record start time and trigger workers */
    int64_t work_start_ns = now_monotonic_ns();

    /* Trigger worker threads to start */
    pthread_barrier_wait(&compute_start_barrier);

    /* Wait for workers to complete */
    pthread_barrier_wait(&compute_done_barrier);

    int64_t work_end_ns = now_monotonic_ns();
    int64_t work_duration_ns = work_end_ns - work_start_ns;

    /* Update EMA of computation duration (clamp to reasonable bounds) */
    ema_duration_ns = ema_alpha * (double)work_duration_ns + (1.0 - ema_alpha) * ema_duration_ns;
    if (ema_duration_ns < 0.0)
      ema_duration_ns = 0.0;
    if (ema_duration_ns > EMA_MAX_NS)
      ema_duration_ns = EMA_MAX_NS;

    /* Calculate drift relative to scheduled_time_ns */
    int64_t schedule_drift_ns = work_end_ns - scheduled_time_ns;

    /* Performance metrics collection and logging */
    double cpu_percent = get_cpu_usage(&cpu_last_time, &cpu_last_usage);
    double memory_mb = get_memory_mb();
    log_system_metrics(current_minute_ms, cpu_percent, memory_mb);
    log_scheduler_metrics(scheduled_time_ns / NS_PER_MS, work_end_ns / NS_PER_MS, schedule_drift_ns);

    /* Schedule next period */
    scheduled_time_ns += PERIOD_NS;
  }

  /* Unblock worker threads so they can exit */
  if (shutdown_requested)
  {
    pthread_barrier_wait(&compute_start_barrier);
    pthread_barrier_wait(&compute_done_barrier);
  }

  return NULL;
}