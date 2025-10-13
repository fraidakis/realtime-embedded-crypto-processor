/**
 * @file websocket.c
 * @brief WebSocket connection and handling implementation
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#include "websocket.h"
#include "okx_parser.h"
#include "../data/queue.h"
#include "../utils/time_utils.h"

/* WebSocket globals */
struct lws_context *lws_context;
struct lws *ws_client = NULL;
int reconnect_attempts, reconnect_backoff_s;

/**
 * @brief Libwebsockets callback function.
 * @param wsi WebSocket instance.
 * @param reason Callback reason.
 * @param user User data.
 * @param in Input data.
 * @param len Input data length.
 * @return 0 on success, -1 on error.
 */
int okx_ws_client_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
  (void)user;

  switch (reason)
  {

  case LWS_CALLBACK_CLIENT_ESTABLISHED:
  {
    /* Connected: send subscription message */
    printf("INFO: WebSocket connection established to OKX\n");

    // Need to allocate buffer with LWS_PRE bytes before the payload
    // LWS_PRE: number of extra bytes to reserve at the start of buffer (header bytes)
    size_t payload_len = strlen(okx_subscribe_payload);
    unsigned char *buf = malloc(LWS_PRE + payload_len);

    if (!buf)
    {
      fprintf(stderr, "ERROR: Failed to allocate buffer for subscription message\n");
      return -1;
    }

    // Copy payload after LWS_PRE bytes
    memcpy(buf + LWS_PRE, okx_subscribe_payload, payload_len);

    // Send the subscription message
    int result = lws_write(wsi, buf + LWS_PRE, payload_len, LWS_WRITE_TEXT);
    free(buf);

    if (result < 0)
    {
      fprintf(stderr, "ERROR: Failed to send subscription message\n");
      return -1;
    }

    ws_client = wsi; // Store the websocket instance globally

    reconnect_attempts = 0;
    reconnect_backoff_s = 2;

    break;
  }

  case LWS_CALLBACK_CLIENT_RECEIVE:
  {
    // Record receive time immediately
    int64_t recv_ts_ms = now_ms();

    // Create minimal message with raw data
    raw_trade_message msg;
    memset(&msg, 0, sizeof(msg));
    msg.receive_ts_ms = recv_ts_ms; // Use the immediate timestamp

    // Copy raw message without parsing (null-terminate)
    size_t copy_len = len < sizeof(msg.raw_json) - 1 ? len : sizeof(msg.raw_json) - 1;
    memcpy(msg.raw_json, (const char *)in, copy_len);
    msg.raw_json[copy_len] = '\0';

    // Push to queue immediately
    trade_queue_push(&raw_queue, &msg);

    break;
  }

  case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
  {
    fprintf(stderr, "ERROR: WebSocket connection failed: %s\n", in ? (char *)in : "Unknown error");
    ws_client = NULL;
    break;
  }

  case LWS_CALLBACK_CLIENT_CLOSED:
  {
    if (shutdown_requested)
      printf("INFO: WebSocket connection closed gracefully\n");
    else
      fprintf(stderr, "WARNING: WebSocket connection lost unexpectedly\n");

    ws_client = NULL;
    break;
  }

  default:
    break;
  }

  return 0;
}

/* lws protocol list */
static const struct lws_protocols ws_protocols[] = {
  {
    .name = "okx-protocol",
    .callback = okx_ws_client_callback,
    .per_session_data_size = 0,
    .rx_buffer_size = 0,
  },
  {
    .name = NULL,
    .callback = NULL,
    .per_session_data_size = 0,
    .rx_buffer_size = 0,
  }
};

/**
 * @brief Thread function to manage websocket connection.
 * @param arg Thread argument (unused).
 * @return NULL.
 */
void *websocket_thread_fn(void *arg)
{
  (void)arg;
  struct lws_context_creation_info ctx_info; // Context creation info
  struct lws_client_connect_info conn_info;    // Connection info

  memset(&ctx_info, 0, sizeof(ctx_info));
  ctx_info.port = CONTEXT_PORT_NO_LISTEN;                  // Define as client only (no server)
  ctx_info.protocols = ws_protocols;                          // Set the protocols
  ctx_info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT; // Initialize SSL (required for wss)

  lws_context = lws_create_context(&ctx_info); // Create the websocket context with above configuration
  if (!lws_context)
  {
    fprintf(stderr, "ERROR: Failed to create WebSocket context\n");
    exit(1);
  }
  
  printf("INFO: WebSocket context created successfully\n");

  const int MAX_RETRY_ATTEMPTS = 8; // 2^9-1 = 511s total wait time (around 8.5 minutes)

  while (!shutdown_requested)
  {
    printf("INFO: Attempting to connect to OKX WebSocket API...\n");
    memset(&conn_info, 0, sizeof(conn_info));
    conn_info.context = lws_context;      // Use the created context
    conn_info.address = "ws.okx.com"; // OKX WebSocket server address
    conn_info.port = 8443;            // SSL WebSocket port
    conn_info.path = "/ws/v5/public"; // OKX public API endpoint
    conn_info.host = conn_info.address;
    conn_info.origin = conn_info.address;
    conn_info.protocol = ws_protocols[0].name;    // Use the defined protocol
    conn_info.ssl_connection = LCCSCF_USE_SSL; // Use SSL
    conn_info.pwsi = &ws_client;              // Pointer to store the websocket instance

    ws_client = lws_client_connect_via_info(&conn_info); // Create the websocket connection
    /* if connection succeeds, callback will be called with* LWS_CALLBACK_CLIENT_ESTABLISHED */

    printf("INFO: Connection attempt initiated, entering service loop...\n");

    /* run service loop until connection closed or established */
    while (lws_service(lws_context, 1000) >= 0 && !shutdown_requested)
    { // 1000 ms timeout (ignored from v3.2)
      if (ws_client == NULL)
      {
        // Connection was closed by the server or an error occurred
        break;
      }
    }

    if (shutdown_requested)
      break; // Break outer loop if signaled (SIGINT)

    if (ws_client == NULL) // Connection failed or was lost
    {
      if (++reconnect_attempts > MAX_RETRY_ATTEMPTS)
      {
        fprintf(stderr, "ERROR: Failed to reconnect after %d attempts, terminating\n", MAX_RETRY_ATTEMPTS);
        raise(SIGINT); // signal main thread to exit
        break;
      }

      fprintf(stderr, "WARNING: Connection failed, retry %d/%d - waiting %ds before next attempt\n", 
              reconnect_attempts, MAX_RETRY_ATTEMPTS, reconnect_backoff_s);
      sleep(reconnect_backoff_s);

      // Exponential backoff
      reconnect_backoff_s = reconnect_backoff_s * 2;
    }
  }

  printf("INFO: WebSocket thread shutting down\n");
  lws_context_destroy(lws_context);
  return NULL;
}