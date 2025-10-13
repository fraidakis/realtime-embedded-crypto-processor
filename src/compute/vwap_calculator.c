/**
 * @file vwap_calculator.c
 * @brief VWAP calculation and worker thread implementation
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#include "vwap_calculator.h"
#include "../data/sliding_window.h"
#include "../data/vwap_history.h"
#include "../logging/logger.h"

/**
 * @brief Worker thread for calculating and logging moving averages (Task 2).
 * @param arg Thread argument (unused).
 * @return NULL.
 */
void *vwap_worker_fn(void *arg)
{
  (void)arg;

  while (!shutdown_requested)
  {
    pthread_barrier_wait(&compute_start_barrier); // Wait for coordinator signal

    if (shutdown_requested)
    {
      // Ensure the coordinator's second barrier wait completes on shutdown
      pthread_barrier_wait(&compute_done_barrier);
      break;
    }

    for (int i = 0; i < NUM_SYMBOLS; ++i)
    {
      double vwap;
      sliding_window_snapshot_vwap(&symbols[i].trade_window, &vwap); // get current VWAP (volume unused)
      vwap_history_append(&symbols[i].vwap_hist, current_minute_ms, vwap); // store in history
      vwap_log_append_csv(i, current_minute_ms, vwap);        // append to file (without volume)
    }

    pthread_barrier_wait(&compute_done_barrier); // Signal completion
  }

  return NULL;
}