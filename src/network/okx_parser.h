/**
 * @file okx_parser.h
 * @brief OKX JSON message parsing declarations
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#ifndef OKX_PARSER_H
#define OKX_PARSER_H

#include "../../include/common.h"

/**
 * @brief Helper function to extract quoted string value (C version).
 * @param json JSON string to parse.
 * @param key Key to search for.
 * @param out Output buffer for the value.
 * @param outsz Output buffer size.
 * @return Pointer to position after the extracted value, or NULL on error.
 */
const char *json_extract_string(const char *json, const char *key, char *out, size_t outsz);

/**
 * @brief Parse OKX trade JSON message.
 * @param json Raw JSON message.
 * @param msg Pointer to raw_trade_message to populate.
 * @return 1 on success, 0 on failure.
 */
int parse_okx_trade(const char *json, raw_trade_message *msg);

extern const char *okx_subscribe_payload;

#endif /* OKX_PARSER_H */