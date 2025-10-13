/**
 * @file queue.h
 * @brief Raw trade queue operations declarations
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#ifndef QUEUE_H
#define QUEUE_H

#include "../../include/common.h"

/**
 * @brief Initializes a raw trade queue.
 * @param q Pointer to the raw_trade_queue structure.
 * @param capacity The maximum number of elements in the queue.
 */
void raw_queue_init(raw_trade_queue *q, uint32_t capacity);

/**
 * @brief Pushes a raw trade message to the queue.
 * @details If the queue is full, the oldest message is overwritten. This is a
 * non-blocking strategy suitable for high-throughput data streams.
 * @param q Pointer to the raw_trade_queue structure.
 * @param msg Pointer to the raw_trade_message to push.
 */
void raw_queue_push(raw_trade_queue *queue, const raw_trade_message *msg_in);

/**
 * @brief Pops a message from the raw trade queue.
 * @details Blocks if the queue is empty until a message is available or shutdown is requested.
 * @param q Pointer to the raw_trade_queue structure.
 * @param out Pointer to a raw_trade_message to store the popped message.
 * @return 1 if a message was popped, 0 if the queue is empty and shutdown is initiated.
 */
int raw_queue_pop(raw_trade_queue *queue, raw_trade_message *msg_out);

/**
 * @brief Cleans up resources used by a raw_trade_queue.
 * @param q Pointer to the raw_trade_queue.
 */
void trade_queue_cleanup(raw_trade_queue *q);

/* Compatibility aliases for renamed queue API */
static inline void trade_queue_init(raw_trade_queue *q, uint32_t capacity) { raw_queue_init(q, capacity); }
static inline void trade_queue_push(raw_trade_queue *q, const raw_trade_message *msg) { raw_queue_push(q, msg); }
static inline int trade_queue_pop(raw_trade_queue *q, raw_trade_message *out) { return raw_queue_pop(q, out); }

#endif /* QUEUE_H */