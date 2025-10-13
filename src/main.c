/**
 * @file main.c
 * @brief Real-time OKX trade data processor for Raspberry Pi - Main entry point.
 *
 * This program connects to the OKX WebSocket API to receive real-time trade data
 * for multiple cryptocurrency pairs. It processes trades using a producer-consumer
 * model, computes volume-weighted average prices (VWAP) over sliding windows,
 * and calculates Pearson correlations between symbols' moving averages.
 *
 * @details
 * ## Key Features:
 * - Multi-threaded architecture with separate threads for WebSocket handling,
 *   event processing, scheduling, and computations.
 * - Efficient in-memory data management using circular buffers for sliding windows.
 * - Per-minute calculation of 15-minute VWAP and total volume.
 * - Per-minute calculation of cross-asset Pearson correlations with lag analysis.
 * - Precise, drift-compensating scheduling using `clock_nanosleep`.
 * - Comprehensive logging of raw trades, computed metrics, and system performance.
 * - Graceful shutdown on SIGINT/SIGTERM.
*
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#include "../include/common.h"
#include "config.h"
#include "data/queue.h"
#include "data/sliding_window.h"
#include "data/vwap_history.h"
#include "utils/time_utils.h"
#include "logging/logger.h"
#include "network/websocket.h"
#include "network/okx_parser.h"
#include "compute/vwap_calculator.h"
#include "compute/correlation.h"
#include "scheduler/scheduler.h"

/* ============================================================================
 * GLOBAL VARIABLE DEFINITIONS
 * ============================================================================ */

/* Array of consolidated symbol data */
symbol_data symbols[NUM_SYMBOLS];

/* Global trade queue and file descriptors */
raw_trade_queue raw_queue;
int latency_log_fd = -1;

/* Worker thread synchronization */
pthread_t vwap_worker_thread;
pthread_t correlation_worker_thread;
pthread_barrier_t compute_start_barrier; // To start workers together
pthread_barrier_t compute_done_barrier;  // To wait for workers to finish
int64_t current_minute_ms;

/* ============================================================================
 * INITIALIZATION AND CLEANUP
 * ============================================================================ */

/**
 * @brief Cleans up all program resources.
 */
static void cleanup_resources(void)
{
  /* cleanup all symbol data structures */
  for (int i = 0; i < NUM_SYMBOLS; ++i)
  {
    if (symbols[i].trade_log_fd >= 0)
    {
      close(symbols[i].trade_log_fd);
      symbols[i].trade_log_fd = -1;
    }
    sliding_window_cleanup(&symbols[i].trade_window);
    vwap_history_cleanup(&symbols[i].vwap_hist);
  }

  if (latency_log_fd >= 0)
    close(latency_log_fd);

  trade_queue_cleanup(&raw_queue); // cleanup raw trade queue resources
  printf("INFO: Resource cleanup complete\n");
}

/**
 * @brief Initialize all symbol data structures.
 */
static void symbols_data_init(void)
{  
  for (int i = 0; i < NUM_SYMBOLS; ++i)
  {
    symbols[i].symbol = SYMBOLS[i];
    symbols[i].trade_log_fd = -1;
    sliding_window_init(&symbols[i].trade_window);
    vwap_history_init(&symbols[i].vwap_hist, VWAP_HISTORY_SIZE_MINUTES);
  }
}

/* ============================================================================
 * TRADE PROCESSING THREAD
 * ============================================================================ */

/**
 * @brief Consumer thread for processing events.
 * @param arg Thread argument (unused).
 * @return NULL.
 */
static void *trade_processor_thread_fn(void *arg)
{
  (void)arg;
  raw_trade_message msg;

  while (!shutdown_requested)
  {
    if (!trade_queue_pop(&raw_queue, &msg))
    {
      if (shutdown_requested)
        break;
      continue;
    }

    /* parse the raw JSON message to extract trade details */
    if (!parse_okx_trade(msg.raw_json, &msg))
    {
      // skip invalid messages - warnings already printed in parse function
      continue;
    }

    /* process message: append to log and update window */
    trade_log_append(msg.symbol_index, &msg);
    int64_t process_ts_ms = now_ms();
    log_latency_metrics(msg.symbol_index, msg.exchange_ts_ms, msg.receive_ts_ms, process_ts_ms);
    sliding_window_add_trade(&symbols[msg.symbol_index].trade_window, msg.exchange_ts_ms, msg.price, msg.size);
  }

  return NULL;
}

/* ============================================================================
 * SIGNAL HANDLING
 * ============================================================================ */

/**
 * @brief Signal handler for graceful shutdown.
 * @param sig Signal number.
 */
static void on_termination_signal(int sig)
{
  (void)sig;
  printf("\n=== GRACEFUL TERMINATION INITIATED ===\n");
  printf("INFO: Received termination signal, shutting down...\n");

  shutdown_requested = 1;

  /* wake up any threads that are blocked on I/O or condition variables */
  lws_cancel_service(lws_context);               // unblocks lws_service
  pthread_cond_signal(&raw_queue.cond_not_empty); // unblocks trade_queue_pop
}

/* ============================================================================
 * MAIN FUNCTION
 * ============================================================================ */

/**
 * @brief Main entry point of the program.
 * @return 0 on success, 1 on error.
 */
int main(void)
{
  printf("=== OKX REAL-TIME TRADE PROCESSOR STARTING ===\n");
  printf("INFO: Monitoring %d cryptocurrency symbols\n", NUM_SYMBOLS);
  printf("INFO: Window size: %d minutes (%lld ms)\n", WINDOW_MINUTES, (long long)WINDOW_MS);
  printf("INFO: Window capacity: %d trades per symbol\n", WINDOW_CAPACITY);
  printf("INFO: Moving average points: %d\n", MOVING_AVG_POINTS);
  printf("INFO: Maximum correlation lag: %d minutes\n", MAX_LAG_MINUTES);
  
  signal(SIGINT, on_termination_signal);
  signal(SIGTERM, on_termination_signal);

  ensure_BASE_DATA_DIRs();

  /* init structures */
  trade_queue_init(&raw_queue, RAW_TRADE_QUEUE_SIZE); // initialize raw trade queue
  symbols_data_init();                       // initialize all symbol data structures

  init_output_files(); // create and initialize all output files

  /* create websocket thread */
  lws_set_log_level(LLL_USER | LLL_ERR | LLL_WARN, NULL); // set lws log level (enable user, error, warning)
  pthread_t websocket_thread;
  if (pthread_create(&websocket_thread, NULL, websocket_thread_fn, NULL) != 0)
  {
    fprintf(stderr, "ERROR: Failed to create WebSocket thread: %s\n", strerror(errno));
    return 1;
  }

  /* create trade processor thread */
  pthread_t trade_processor_thread;
  if (pthread_create(&trade_processor_thread, NULL, trade_processor_thread_fn, NULL) != 0)
  {
    fprintf(stderr, "ERROR: Failed to create trade processor thread: %s\n", strerror(errno));
    return 1;
  }

  /* initialize barriers for 3 threads: coordinator + 2 workers */
  pthread_barrier_init(&compute_start_barrier, NULL, 3);
  pthread_barrier_init(&compute_done_barrier, NULL, 3);

  /* create worker threads */
  if (pthread_create(&vwap_worker_thread, NULL, vwap_worker_fn, NULL) != 0)
  {
    fprintf(stderr, "ERROR: Failed to create VWAP worker thread: %s\n", strerror(errno));
    return 1;
  }
  if (pthread_create(&correlation_worker_thread, NULL, correlation_worker_fn, NULL) != 0)
  {
    fprintf(stderr, "ERROR: Failed to create correlation worker thread: %s\n", strerror(errno));
    return 1;
  }

  /* create metrics coordinator thread */
  pthread_t scheduler_thread;
  if (pthread_create(&scheduler_thread, NULL, scheduler_thread_fn, NULL) != 0)
  {
    fprintf(stderr, "ERROR: Failed to create scheduler thread: %s\n", strerror(errno));
    return 1;
  }

  printf("=== ALL THREADS STARTED SUCCESSFULLY ===\n");
  printf("INFO: System is now processing real-time trade data\n");
  printf("INFO: Press Ctrl+C to stop gracefully\n");

  pthread_join(websocket_thread, NULL);
  pthread_join(trade_processor_thread, NULL);
  pthread_join(scheduler_thread, NULL);
  pthread_join(vwap_worker_thread, NULL);
  pthread_join(correlation_worker_thread, NULL);

  printf("INFO: All threads have terminated\n");

  pthread_barrier_destroy(&compute_start_barrier);
  pthread_barrier_destroy(&compute_done_barrier);

  /* cleanup */
  printf("INFO: Cleaning up resources...\n");
  cleanup_resources();

  printf("=== PROGRAM TERMINATED GRACEFULLY ===\n");
  return 0;
}