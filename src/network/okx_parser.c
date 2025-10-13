/**
 * @file okx_parser.c
 * @brief OKX JSON message parsing implementation
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#include "okx_parser.h"
#include "../utils/time_utils.h"

const char *okx_subscribe_payload =
    "{"
    "\"op\":\"subscribe\"," // JSON -> "op":"subscribe",
    "\"args\":["
    "{\"channel\":\"trades\",\"instId\":\"BTC-USDT\"},"
    "{\"channel\":\"trades\",\"instId\":\"ADA-USDT\"},"
    "{\"channel\":\"trades\",\"instId\":\"ETH-USDT\"},"
    "{\"channel\":\"trades\",\"instId\":\"DOGE-USDT\"},"
    "{\"channel\":\"trades\",\"instId\":\"XRP-USDT\"},"
    "{\"channel\":\"trades\",\"instId\":\"SOL-USDT\"},"
    "{\"channel\":\"trades\",\"instId\":\"LTC-USDT\"},"
    "{\"channel\":\"trades\",\"instId\":\"BNB-USDT\"}"
    "]"
    "}";

/**
 * @brief Helper function to extract quoted string value (C version).
 * @param json JSON string to parse.
 * @param key Key to search for.
 * @param out Output buffer for the value.
 * @param outsz Output buffer size.
 * @return Pointer to position after the extracted value, or NULL on error.
 */
const char *json_extract_string(const char *json, const char *key, char *out, size_t outsz)
{
  const char *p = strstr(json, key);
  if (!p)
    return NULL;

  // Skip to ':'
  p = strchr(p, ':');
  if (!p)
    return NULL;

  // Skip whitespace and find opening quote
  p++;
  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
    p++;
  if (*p != '"')
    return NULL;
  p++; // Skip the quote

  // Find closing quote
  const char *end = strchr(p, '"');
  if (!end)
    return NULL;

  // Extract value, truncate if too long
  size_t len = end - p;
  if (len >= outsz)
    len = outsz - 1;
  memcpy(out, p, len);
  out[len] = '\0';

  return end + 1; // Return position after closing quote
}

/**
 * @brief Parse OKX trade JSON message.
 * 
 * OKX public trade message format (example):
 *   {
 *   "arg": {
 *       "channel": "trades",
 *       "instType": "SPOT",
 *       "instId": "BTC-USDT"
 *   },
 *   "data": [
 *       {
 *       "instId": "BTC-USDT",
 *       "px": "27340.8",
 *       "sz": "0.0005",
 *       "side": "sell",
 *       "ts": "1694464949239"
 *       }
 *   ]
 *   }
 * 
 * @param json Raw JSON message.
 * @param msg Pointer to raw_trade_message to populate.
 * @return 1 on success, 0 on failure.
 */
int parse_okx_trade(const char *json, raw_trade_message *msg)
{
  // Find the "data" array first
  const char *data_arr_start = strstr(json, "\"data\"");
  if (!data_arr_start) {
    return 0;
  }

  data_arr_start = strchr(data_arr_start, '[');
  if (!data_arr_start) {
    fprintf(stderr, "WARNING: Invalid trade message - malformed 'data' array\n");
    return 0;
  }
  data_arr_start++; // Skip '['

  // Find the first trade object
  const char *trade_obj_start = strchr(data_arr_start, '{');
  if (!trade_obj_start) {
    fprintf(stderr, "WARNING: Invalid trade message - no trade object found\n");
    return 0;
  }

  // Sequential parsing with fallbacks
  char inst_id[32];
  const char *cursor = json_extract_string(trade_obj_start, "\"instId\"", inst_id, sizeof(inst_id));
  if (!cursor) {
    fprintf(stderr, "WARNING: Failed to parse instId from trade message\n");
    return 0;
  }

  // Map instId to symbol index
  int symbol_idx = -1;
  for (int i = 0; i < NUM_SYMBOLS; ++i)
  {
    if (strcmp(inst_id, SYMBOLS[i]) == 0)
    {
      symbol_idx = i;
      break;
    }
  }
  if (symbol_idx < 0) {
    fprintf(stderr, "WARNING: Unknown symbol '%s' in trade message\n", inst_id);
    return 0;
  }

  // Extract price with validation
  char price_str[32];
  cursor = json_extract_string(cursor, "\"px\"", price_str, sizeof(price_str));
  if (!cursor) {
    fprintf(stderr, "WARNING: Failed to parse price from trade message for %s\n", inst_id);
    return 0;
  }

  char *endp;
  errno = 0;
  double price = strtod(price_str, &endp);
  if (errno != 0 || *endp != '\0' || price <= 0) {
    fprintf(stderr, "WARNING: Invalid price value '%s' for symbol %s\n", price_str, inst_id);
    return 0;
  }

  // Extract size with validation
  char size_str[32];
  cursor = json_extract_string(cursor, "\"sz\"", size_str, sizeof(size_str));
  if (!cursor) {
    fprintf(stderr, "WARNING: Failed to parse size from trade message for %s\n", inst_id);
    return 0;
  }

  errno = 0;
  double size = strtod(size_str, &endp);
  if (errno != 0 || *endp != '\0' || size <= 0) {
    fprintf(stderr, "WARNING: Invalid size value '%s' for symbol %s\n", size_str, inst_id);
    return 0;
  }

  // Extract timestamp with validation
  char ts_str[32];
  cursor = json_extract_string(cursor, "\"ts\"", ts_str, sizeof(ts_str));
  int64_t ts_ms = 0;
  if (cursor)
  {
    errno = 0;
    ts_ms = strtoll(ts_str, &endp, 10);
    if (errno != 0 || *endp != '\0' || ts_ms <= 0)
    {
      fprintf(stderr, "WARNING: Invalid timestamp '%s' for %s, using current time\n", ts_str, inst_id);
      ts_ms = now_ms(); // Fallback to current time
    }
  }
  else
  {
    fprintf(stderr, "WARNING: Missing timestamp for %s, using current time\n", inst_id);
    ts_ms = now_ms();
  }

  // Populate the event structure
  msg->symbol_index = symbol_idx;
  msg->exchange_ts_ms = ts_ms;
  msg->price = price;
  msg->size = size;

  return 1;
}