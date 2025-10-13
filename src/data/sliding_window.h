/**
 * @file sliding_window.h
 * @brief Sliding window operations declarations
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#ifndef SLIDING_WINDOW_H
#define SLIDING_WINDOW_H

#include "../../include/common.h"

/**
 * @brief Initializes a sliding_window structure.
 * @param w Pointer to the sliding_window.
 */
void sliding_window_init(sliding_window *w);

/**
 * @brief Pushes a new trade to the sliding window.
 * @details Prunes old trades that fall outside the `WINDOW_MS` duration and updates
 * the running sums for price-volume and total volume.
 * @param w Pointer to the sliding_window.
 * @param ts_ms Timestamp of the new trade.
 * @param price Price of the new trade.
 * @param size Size of the new trade.
 */
void sliding_window_add_trade(sliding_window *w, int64_t ts_ms, double price, double size);

/**
 * @brief Takes a snapshot of the current VWAP and total volume from the window.
 * @param w Pointer to the sliding_window.
 * @param out_vwap Pointer to store the calculated VWAP.
 */
void sliding_window_snapshot_vwap(sliding_window *w, double *out_vwap);

/**
 * @brief Cleans up resources used by a sliding_window.
 * @param w Pointer to the sliding_window.
 */
void sliding_window_cleanup(sliding_window *w);

#endif /* SLIDING_WINDOW_H */
