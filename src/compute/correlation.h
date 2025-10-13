/**
 * @file correlation.h
 * @brief Correlation calculation and worker thread declarations
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#ifndef CORRELATION_H
#define CORRELATION_H

#include "../../include/common.h"

/**
 * @brief Computes the Pearson correlation coefficient between two data series.
 * @param x Pointer to the first data array.
 * @param y Pointer to the second data array.
 * @param n The number of points in each array.
 * @return The correlation coefficient, or NAN if the denominator is zero.
 */
double pearson_correlation(const double *x, const double *y, int n);

/**
 * @brief Finds the best correlation of a source vector against a target history.
 * @details Searches for the time-lagged window in `target_hist` that has the highest
 * absolute Pearson correlation with `src_vec`.
 * @param src_vec The source vector of VWAP data.
 * @param target_hist The moving history to search within.
 * @param window_len The number of points in the vectors.
 * @param min_offset_min The minimum lag to consider (to avoid self-correlation).
 * @param max_lag_min The maximum lag to search.
 * @param out_corr Pointer to store the best correlation coefficient.
 * @param out_minute_ts_ms Pointer to store the timestamp of the best correlation.
 */
void find_best_lagged_correlation(const double *src_vec, vwap_history *target_hist, int window_len, int min_offset_min,
                                  int max_lag_min, double *out_corr, int64_t *out_minute_ts_ms);

/**
 * @brief Worker thread for calculating and logging correlations (Task 3).
 * @param arg Thread argument (unused).
 * @return NULL.
 */
void *correlation_worker_fn(void *arg);

#endif /* CORRELATION_H */