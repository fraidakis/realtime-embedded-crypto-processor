/**
 * @file common.h
 * @brief Common definitions and includes for the OKX real-time trade processor.
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#ifndef COMMON_H
#define COMMON_H

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <libwebsockets.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/* ============================================================================
 * CONFIGURATION AND CONSTANTS
 * ============================================================================ */

/**
 * @brief Number of cryptocurrency symbols to monitor.
 */
#define NUM_SYMBOLS 8

/**
 * @brief Array of symbol names (e.g., "BTC-USDT").
 */
extern const char *SYMBOLS[NUM_SYMBOLS];

/* Data directories for logging and metrics */
#define BASE_DATA_DIR "data"
#define TRADES_LOG_DIR "data/trades"
#define METRICS_DIR "data/metrics"
#define VWAP_DIR "data/metrics/vwap"
#define CORRELATION_DIR "data/metrics/correlations"
#define PERFORMANCE_LOGS_DIR "data/performance"

/* Time window and history sizes */
#define WINDOW_MINUTES 15                        /**< 15-minute sliding window for trades */
#define WINDOW_MS (WINDOW_MINUTES * 60 * 1000LL) /**< Window duration in milliseconds */
#define WINDOW_CAPACITY 50000                    /**< Maximum trades in sliding window per symbol */

/* History for moving averages and correlations */
#define MOVING_AVG_POINTS 8                                          /**< Number of recent points for correlation analysis */
#define MAX_LAG_MINUTES 60                                           /**< Maximum lag (minutes) to search for correlations */
#define VWAP_HISTORY_SIZE_MINUTES (MAX_LAG_MINUTES + MOVING_AVG_POINTS) /**< Number of moving averages to keep in memory per symbol */

/* Event queue capacity */
#define RAW_TRADE_QUEUE_SIZE 1024 /**< Capacity of the raw trade queue */

/* Synchronization settings */
#define FSYNC_PER_WRITE 0 /**< Set to 1 for fsync on every write (durability but slower) */

/* Time conversion constants */
#define NS_PER_MS 1000000LL
#define NS_PER_SEC 1000000000LL
#define MS_PER_MINUTE 60000LL
#define NS_PER_MINUTE (MS_PER_MINUTE * NS_PER_MS)

/* Global flags */
extern int shutdown_requested; /**< Flag to signal graceful shutdown on SIGINT */

/* ============================================================================
 * CORE DATA STRUCTURES
 * ============================================================================ */

/**
 * @brief Raw trade message received from WebSocket with metadata.
 */
typedef struct
{
  int symbol_index;       /**< Index in the global SYMBOLS array. */
  int64_t exchange_ts_ms; /**< Exchange-provided trade timestamp (milliseconds). */
  double price;           /**< Trade price. */
  double size;            /**< Trade size/volume. */
  char raw_json[1024];    /**< Raw JSON message for logging. Assumes messages fit. */
  int64_t receive_ts_ms;  /**< Local timestamp when the message was received. */
} raw_trade_message;

/**
 * @brief A processed trade record stored within the sliding window.
 */
typedef struct
{
  int64_t trade_ts_ms;
  double price;
  double size;
} processed_trade;

/**
 * @brief A single data point in the moving average history, representing one minute's VWAP and volume.
 */
typedef struct
{
  int64_t minute_ts_ms; /**< minute timestamp (aligned to minute) */
  double vwap;          /**< VWAP over WINDOW_MS ending at this minute */
} vwap_point;

/* ============================================================================
 * DATA STRUCTURE DEFINITIONS
 * ============================================================================ */

/**
 * @brief A thread-safe, bounded, circular queue for raw trade messages.
 */
struct raw_trade_queue
{
  raw_trade_message *buffer; /**< buffer to store raw trade messages */
  uint32_t capacity;
  uint32_t head_idx, tail_idx;
  pthread_mutex_t lock;          /**< mutex for thread safety (producer-consumer) */
  pthread_cond_t cond_not_empty; /**< condition variable to signal non-empty queue */
};
typedef struct raw_trade_queue raw_trade_queue;

/**
 * @brief A circular buffer for a sliding window of trades, with running sums for O(1) VWAP calculation.
 */
struct sliding_window
{
  processed_trade *buffer; /**< pre-allocated circular buffer */
  uint32_t capacity;      /**< buffer size (e.g., 20000) */
  uint32_t head_idx;          /**< oldest valid entry */
  uint32_t tail_idx;          /**< next insertion point */
  uint32_t size;              /**< number of valid entries */
  double sum_price_volume;    /**< running sum of price * size */
  double sum_volume;          /**< running sum of size */
  pthread_mutex_t lock;
};
typedef struct sliding_window sliding_window;

/**
 * @brief A circular buffer to store the history of per-minute VWAP and volume data points.
 */
struct vwap_history
{
  vwap_point *buffer;
  int capacity;
  int head_idx;  /**< oldest entry index */
  int tail_idx;  /**< next insertion point */
  int size;      /**< current number of entries */
  pthread_mutex_t lock;
};
typedef struct vwap_history vwap_history;

/**
 * @brief A consolidated data structure holding all real-time and historical data for a single symbol.
 */
struct symbol_data
{
  const char *symbol;       /**< symbol name (e.g., "BTC-USDT") */
  sliding_window trade_window;    /**< sliding window for trades */
  vwap_history vwap_hist;         /**< moving average history */
  int trade_log_fd;               /**< file descriptor for trade log */
};
typedef struct symbol_data symbol_data;

/* Global data arrays */
extern symbol_data symbols[NUM_SYMBOLS];
extern raw_trade_queue raw_queue;
extern int latency_log_fd;

/* Worker thread synchronization */
extern pthread_t vwap_worker_thread;
extern pthread_t correlation_worker_thread;
extern pthread_barrier_t compute_start_barrier;
extern pthread_barrier_t compute_done_barrier;
extern int64_t current_minute_ms;

/* WebSocket globals */
extern struct lws_context *lws_context;
extern struct lws *ws_client;

#endif /* COMMON_H */