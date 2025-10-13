/**
 * @file scheduler.h
 * @brief Scheduler thread declarations
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../../include/common.h"

/**
 * @brief Coordinator thread that schedules the worker threads to run precisely every minute.
 * @param arg Thread argument (unused).
 * @return NULL.
 */
void *scheduler_thread_fn(void *arg);

#endif /* SCHEDULER_H */