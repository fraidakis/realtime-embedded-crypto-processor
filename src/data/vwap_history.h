/**
 * @file vwap_history.h
 * @brief VWAP history operations declarations
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#ifndef VWAP_HISTORY_H
#define VWAP_HISTORY_H

#include "../../include/common.h"

/**
 * @brief Initializes a vwap_history structure.
 * @param h Pointer to the vwap_history.
 * @param capacity The maximum number of history points to store.
 */
void vwap_history_init(vwap_history *h, int capacity);

/**
 * @brief Push new moving point to history (overwrites oldest if full).
 * @param h Pointer to the vwap_history.
 * @param minute_ts_ms Minute timestamp.
 * @param vwap VWAP value.
 */
void vwap_history_append(vwap_history *h, int64_t minute_ts_ms, double vwap);

/**
 * @brief Get last n moving points from history.
 * @param h Pointer to the vwap_history.
 * @param n Number of points to retrieve.
 * @param out Output buffer for points.
 * @return 1 if successful, 0 if not enough data.
 */
int vwap_history_get_recent(vwap_history *h, int n, vwap_point *out);

/**
 * @brief Cleans up resources used by a vwap_history.
 * @param h Pointer to the vwap_history.
 */
void vwap_history_cleanup(vwap_history *h);

#endif /* VWAP_HISTORY_H */