/**
 * @file sliding_window.c
 * @brief Sliding window operations implementation
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#include "sliding_window.h"

/**
 * @brief Initializes a sliding_window structure.
 * @param w Pointer to the sliding_window.
 */
void sliding_window_init(sliding_window *w)
{
  w->buffer = calloc(WINDOW_CAPACITY, sizeof(processed_trade));

  if (!w->buffer)
  {
    fprintf(stderr, "ERROR: Failed to allocate trade window buffer for %d trades (%.2f MB)\n", 
            WINDOW_CAPACITY, (WINDOW_CAPACITY * sizeof(processed_trade)) / (1024.0 * 1024.0));
    exit(1);
  }

  w->capacity = WINDOW_CAPACITY;
  w->head_idx = w->tail_idx = w->size = 0;
  w->sum_price_volume = 0.0;
  w->sum_volume = 0.0;
  pthread_mutex_init(&w->lock, NULL);
}

/**
 * @brief Pushes a new trade to the sliding window.
 * @details Prunes old trades that fall outside the `WINDOW_MS` duration and updates
 * the running sums for price-volume and total volume.
 * @param w Pointer to the sliding_window.
 * @param ts_ms Timestamp of the new trade.
 * @param price Price of the new trade.
 * @param size Size of the new trade.
 */
void sliding_window_add_trade(sliding_window *w, int64_t ts_ms, double price, double size)
{
  pthread_mutex_lock(&w->lock);

  // 1. Prune old entries from head (O(k) where k = expired entries, typically small)
  int64_t expiry_cutoff_ms = ts_ms - WINDOW_MS;
  while (w->size > 0 && w->buffer[w->head_idx].trade_ts_ms < expiry_cutoff_ms)
  {
    w->sum_price_volume -= w->buffer[w->head_idx].price * w->buffer[w->head_idx].size;
    w->sum_volume -= w->buffer[w->head_idx].size;
    w->head_idx = (w->head_idx + 1) % w->capacity;
    w->size--;
  }

  // 2. Handle buffer full (overwrite oldest if necessary)
  if (w->size == w->capacity)
  {
    // Remove oldest entry
    w->sum_price_volume -= w->buffer[w->head_idx].price * w->buffer[w->head_idx].size;
    w->sum_volume -= w->buffer[w->head_idx].size;
    w->head_idx = (w->head_idx + 1) % w->capacity;
    w->size--;
  }

  // 3. Add new entry
  w->buffer[w->tail_idx].trade_ts_ms = ts_ms;
  w->buffer[w->tail_idx].price = price;
  w->buffer[w->tail_idx].size = size;
  w->tail_idx = (w->tail_idx + 1) % w->capacity;
  w->size++;

  // 4. Update running sums
  w->sum_price_volume += price * size;
  w->sum_volume += size;

  pthread_mutex_unlock(&w->lock);
}

/**
 * @brief Takes a snapshot of the current VWAP and total volume from the window.
 * @param w Pointer to the sliding_window.
 * @param out_vwap Pointer to store the calculated VWAP.
 */
void sliding_window_snapshot_vwap(sliding_window *w, double *out_vwap)
{
  pthread_mutex_lock(&w->lock);

  if (w->sum_volume > 0) // if we have trades in window (volume)
    *out_vwap = w->sum_price_volume / w->sum_volume;
  else
    *out_vwap = NAN;

  pthread_mutex_unlock(&w->lock);
}

/**
 * @brief Cleans up resources used by a sliding_window.
 * @param w Pointer to the sliding_window.
 */
void sliding_window_cleanup(sliding_window *w)
{
  if (w->buffer)
  {
    free(w->buffer);
    w->buffer = NULL;
  }
  pthread_mutex_destroy(&w->lock);
}