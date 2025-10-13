/**
 * @file vwap_history.c
 * @brief VWAP history operations implementation
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#include "vwap_history.h"

/**
 * @brief Initializes a vwap_history structure.
 * @param h Pointer to the vwap_history.
 * @param capacity The maximum number of history points to store.
 */
void vwap_history_init(vwap_history *h, int capacity)
{
  h->buffer = calloc(capacity, sizeof(vwap_point));

  if (!h->buffer)
  {
    fprintf(stderr, "ERROR: Failed to allocate VWAP history buffer for %d points (%.2f KB)\n", 
            capacity, (capacity * sizeof(vwap_point)) / 1024.0);
    exit(1);
  }

  h->capacity = capacity;
  h->head_idx = 0;
  h->tail_idx = 0;
  h->size = 0;
  pthread_mutex_init(&h->lock, NULL);
}

/**
 * @brief Push new moving point to history (overwrites oldest if full).
 * @param h Pointer to the vwap_history.
 * @param minute_ts_ms Minute timestamp.
 * @param vwap VWAP value.
 */
void vwap_history_append(vwap_history *h, int64_t minute_ts_ms, double vwap)
{
  pthread_mutex_lock(&h->lock);

  // Handle buffer full
  if (h->size == h->capacity)
  {
    h->head_idx = (h->head_idx + 1) % h->capacity;
    h->size--;
  }

  // Add new entry
  h->buffer[h->tail_idx].minute_ts_ms = minute_ts_ms;
  h->buffer[h->tail_idx].vwap = vwap;
  h->tail_idx = (h->tail_idx + 1) % h->capacity;
  h->size++;

  pthread_mutex_unlock(&h->lock);
}

/**
 * @brief Get last n moving points from history.
 * @param h Pointer to the vwap_history.
 * @param n Number of points to retrieve.
 * @param out Output buffer for points.
 * @return 1 if successful, 0 if not enough data.
 */
int vwap_history_get_recent(vwap_history *h, int n, vwap_point *out)
{
  pthread_mutex_lock(&h->lock);

  if (h->size < n)
  {
    pthread_mutex_unlock(&h->lock);
    return 0;
  }

  // Start from n entries back from tail
  int start_idx_from_tail = (h->tail_idx - n + h->capacity) % h->capacity;
  for (int i = 0; i < n; ++i)
  {
    int ring_idx = (start_idx_from_tail + i) % h->capacity;
    out[i] = h->buffer[ring_idx];
  }

  pthread_mutex_unlock(&h->lock);

  return 1;
}

/**
 * @brief Cleans up resources used by a vwap_history.
 * @param h Pointer to the vwap_history.
 */
void vwap_history_cleanup(vwap_history *h)
{
  if (h->buffer)
  {
    free(h->buffer);
    h->buffer = NULL;
  }
  pthread_mutex_destroy(&h->lock);
}