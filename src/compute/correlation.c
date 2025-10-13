/**
 * @file correlation.c
 * @brief Correlation calculation and worker thread implementation
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#include "correlation.h"
#include "../data/vwap_history.h"
#include "../logging/logger.h"

/**
 * @brief Computes the Pearson correlation coefficient between two data series.
 * @param x Pointer to the first data array.
 * @param y Pointer to the second data array.
 * @param n The number of points in each array.
 * @return The correlation coefficient, or NAN if the denominator is zero.
 */
double pearson_correlation(const double *x, const double *y, int n)
{
  double sum_x = 0, sum_y = 0, sum_xx = 0, sum_yy = 0, sum_xy = 0;
  for (int i = 0; i < n; ++i)
  {
    sum_x += x[i];
    sum_y += y[i];
    sum_xx += x[i] * x[i];
    sum_yy += y[i] * y[i];
    sum_xy += x[i] * y[i];
  }
  double numerator = n * sum_xy - sum_x * sum_y;
  double denominator = sqrt((n * sum_xx - sum_x * sum_x) * (n * sum_yy - sum_y * sum_y));
  if (denominator == 0)
    return NAN;
  return numerator / denominator;
}

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
                                  int max_lag_min, double *out_corr, int64_t *out_minute_ts_ms)
{
  *out_corr = NAN;
  *out_minute_ts_ms = 0;

  pthread_mutex_lock(&target_hist->lock);

  int hist_len = target_hist->size; // was target_hist->len

  /* need at least (points + min_offset) data points for one comparison */
  if (hist_len < window_len + min_offset_min)
  {
    pthread_mutex_unlock(&target_hist->lock);
    return;
  }

  /* max offset = how many full points windows we can fit in history */
  int max_offset_min = (hist_len - window_len);

  /* limit offset to max_lag_minutes */
  int max_search_offset = max_lag_min < max_offset_min ? max_lag_min : max_offset_min;

  double best_corr_val = 0.0;
  int64_t best_end_minute_ms = 0;
  int found_match = 0;

  for (int offset = min_offset_min; offset <= max_search_offset; ++offset)
  {
    /* window start index = head + (count - points - offset) */
    int window_start_idx = (target_hist->head_idx + target_hist->size - window_len - offset) % target_hist->capacity; // was target_hist->start + target_hist->len
    double target_vec[MOVING_AVG_POINTS];

    /* extract target vector for current offset */
    for (int i = 0; i < window_len; ++i)
    {
      int ring_idx = (window_start_idx + i) % target_hist->capacity;
      target_vec[i] = target_hist->buffer[ring_idx].vwap;
    }

    double corr = pearson_correlation(src_vec, target_vec, window_len);

    if (!isnan(corr)) // denominator not zero
    {
      if (!found_match || fabs(corr) > fabs(best_corr_val)) // better correlation (abs)
      {
        best_corr_val = corr;

        /* minute timestamp is the end of the window */
        int end_idx = (window_start_idx + window_len - 1) % target_hist->capacity;
        best_end_minute_ms = target_hist->buffer[end_idx].minute_ts_ms;

        found_match = 1;
      }
    }
  }

  pthread_mutex_unlock(&target_hist->lock);

  if (found_match)
  {
    *out_corr = best_corr_val;
    *out_minute_ts_ms = best_end_minute_ms;
  }
}

/**
 * @brief Worker thread for calculating and logging correlations (Task 3).
 * @param arg Thread argument (unused).
 * @return NULL.
 */
void *correlation_worker_fn(void *arg)
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

    double src_vwap_vec[MOVING_AVG_POINTS];

    for (int i = 0; i < NUM_SYMBOLS; ++i)
    {
      vwap_point src_points_buf[MOVING_AVG_POINTS];

      /* get last MOVING_AVG_POINTS from history */
      if (!vwap_history_get_recent(&symbols[i].vwap_hist, MOVING_AVG_POINTS, src_points_buf))
        continue; // not enough data

      for (int k = 0; k < MOVING_AVG_POINTS; ++k)
        src_vwap_vec[k] = src_points_buf[k].vwap; // extract VWAPs

      double best_corr_for_symbol = 0.0;
      int64_t best_ts_for_symbol = 0;
      int best_j = -1;
      int found_any = 0;

      for (int j = 0; j < NUM_SYMBOLS; ++j)
      {
        double current_best_corr;
        int64_t current_best_ts;
        int min_offset_min = 0;

        if (i == j)
        {
          /* Same symbol: the first non-overlapping window is after MOVING_AVG_POINTS minutes ago */
          min_offset_min = MOVING_AVG_POINTS;
        }

        find_best_lagged_correlation(src_vwap_vec, &symbols[j].vwap_hist, MOVING_AVG_POINTS, min_offset_min, MAX_LAG_MINUTES, &current_best_corr, &current_best_ts);

        if (!isnan(current_best_corr))
        {
          if (!found_any || fabs(current_best_corr) > fabs(best_corr_for_symbol))
          {
            best_corr_for_symbol = current_best_corr;
            best_ts_for_symbol = current_best_ts;
            best_j = j;
            found_any = 1;
          }
        }
      }

      if (found_any)
      {
        correlation_log_append_csv(i, current_minute_ms, symbols[best_j].symbol, best_corr_for_symbol, best_ts_for_symbol);
      }
    }

    pthread_barrier_wait(&compute_done_barrier); // Signal completion
  }

  return NULL;
}