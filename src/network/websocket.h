/**
 * @file websocket.h
 * @brief WebSocket connection and handling declarations
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "../../include/common.h"

/* WebSocket globals */
extern struct lws_context *lws_context;
extern struct lws *ws_client;
extern int reconnect_attempts, reconnect_backoff_s;

/**
 * @brief Libwebsockets callback function.
 * @param wsi WebSocket instance.
 * @param reason Callback reason.
 * @param user User data.
 * @param in Input data.
 * @param len Input data length.
 * @return 0 on success, -1 on error.
 */
int okx_ws_client_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

/**
 * @brief Thread function to manage websocket connection.
 * @param arg Thread argument (unused).
 * @return NULL.
 */
void *websocket_thread_fn(void *arg);

#endif /* WEBSOCKET_H */