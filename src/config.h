/**
 * @file config.h
 * @brief Configuration constants and global symbol definitions
 *
 * @author Fraidakis Ioannis
 * @date September 2025
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "../include/common.h"

/**
 * @brief Array of symbol names (e.g., "BTC-USDT").
 */
const char *SYMBOLS[NUM_SYMBOLS] = {
    "BTC-USDT", "ADA-USDT", "ETH-USDT",
    "DOGE-USDT", "XRP-USDT", "SOL-USDT",
    "LTC-USDT", "BNB-USDT"
};

/* Global flags */
int shutdown_requested = 0; /**< Flag to signal graceful shutdown on SIGINT */

#endif /* CONFIG_H */