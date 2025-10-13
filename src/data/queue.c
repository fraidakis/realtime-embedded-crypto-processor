/**
 * @file queue.c
 * @brief Raw trade queue operations implementation
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#include "queue.h"

/**
 * @brief Initializes a raw trade queue.
 * @param q Pointer to the raw_trade_queue structure.
 * @param capacity The maximum number of elements in the queue.
 */
void raw_queue_init(raw_trade_queue *q, uint32_t capacity)
{
  q->buffer = calloc(capacity, sizeof(raw_trade_message)); // Allocate buffer

  if (!q->buffer)
  {
    fprintf(stderr, "ERROR: Failed to allocate ring queue buffer for %u messages (%.2f MB)\n", 
            capacity, (capacity * sizeof(raw_trade_message)) / (1024.0 * 1024.0));
    exit(1);
  }

  q->capacity = capacity;
  q->head_idx = q->tail_idx = 0;
  pthread_mutex_init(&q->lock, NULL);
  pthread_cond_init(&q->cond_not_empty, NULL);
}

/**
 * @brief Pushes a raw trade message to the queue.
 * @details If the queue is full, the oldest message is overwritten. This is a
 * non-blocking strategy suitable for high-throughput data streams.
 * @param q Pointer to the raw_trade_queue structure.
 * @param msg Pointer to the raw_trade_message to push.
 */
void raw_queue_push(raw_trade_queue *queue, const raw_trade_message *msg_in)
{
  pthread_mutex_lock(&queue->lock);

  while (((queue->tail_idx + 1) % queue->capacity) == queue->head_idx)
  {
    // queue full: drop oldest trade
    queue->head_idx = (queue->head_idx + 1) % queue->capacity;
  }

  queue->buffer[queue->tail_idx] = *msg_in;
  queue->tail_idx = (queue->tail_idx + 1) % queue->capacity;
  pthread_cond_signal(&queue->cond_not_empty);

  pthread_mutex_unlock(&queue->lock);
}

/**
 * @brief Pops a message from the raw trade queue.
 * @details Blocks if the queue is empty until a message is available or shutdown is requested.
 * @param q Pointer to the raw_trade_queue structure.
 * @param out Pointer to a raw_trade_message to store the popped message.
 * @return 1 if a message was popped, 0 if the queue is empty and shutdown is initiated.
 */
int raw_queue_pop(raw_trade_queue *queue, raw_trade_message *msg_out)
{
  pthread_mutex_lock(&queue->lock);

  while (queue->head_idx == queue->tail_idx && !shutdown_requested)
  { // Check if queue is empty
    pthread_cond_wait(&queue->cond_not_empty, &queue->lock);
  }

  if (shutdown_requested && queue->head_idx == queue->tail_idx)
  {
    pthread_mutex_unlock(&queue->lock);
    return 0; // Queue is empty and we are exiting
  }

  *msg_out = queue->buffer[queue->head_idx];
  queue->head_idx = (queue->head_idx + 1) % queue->capacity;

  pthread_mutex_unlock(&queue->lock);

  return 1;
}

/**
 * @brief Cleans up resources used by a raw_trade_queue.
 * @param q Pointer to the raw_trade_queue.
 */
void trade_queue_cleanup(raw_trade_queue *q)
{
  if (q->buffer)
  {
    free(q->buffer);
    q->buffer = NULL;
  }
  pthread_mutex_destroy(&q->lock);
  pthread_cond_destroy(&q->cond_not_empty);
}