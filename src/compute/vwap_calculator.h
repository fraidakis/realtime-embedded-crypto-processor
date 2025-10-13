/**
 * @file vwap_calculator.h
 * @brief VWAP calculation and worker thread declarations
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#ifndef VWAP_CALCULATOR_H
#define VWAP_CALCULATOR_H

#include "../../include/common.h"

/**
 * @brief Worker thread for calculating and logging moving averages (Task 2).
 * @param arg Thread argument (unused).
 * @return NULL.
 */
void *vwap_worker_fn(void *arg);

#endif /* VWAP_CALCULATOR_H */