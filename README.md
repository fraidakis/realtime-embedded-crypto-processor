# OKX Real-time Trade Processor

A high-performance, real-time cryptocurrency trade data processor designed for Raspberry Pi. This application connects to the OKX WebSocket API to receive live trade data, processes it using a multi-threaded architecture, and performs real-time analytics including VWAP calculations and correlation analysis.

## Project Structure

The project has been modularized for better maintainability and organization:

```
├── src/
│   ├── main.c              # Main entry point and thread coordination
│   ├── config.h            # Configuration constants and global symbols
│   ├── utils/              # Utility functions
│   │   ├── time_utils.c    # Time-related utilities
│   │   ├── time_utils.h    
│   │   ├── system_monitor.c # CPU and memory monitoring
│   │   └── system_monitor.h
│   ├── data/               # Data structures and management
│   │   ├── structures.h    # Core data structure definitions
│   │   ├── queue.c         # Thread-safe message queue
│   │   ├── queue.h
│   │   ├── sliding_window.c # Sliding window for trade data
│   │   ├── sliding_window.h
│   │   ├── vwap_history.c  # VWAP history management
│   │   └── vwap_history.h
│   ├── logging/            # Logging and file I/O
│   │   ├── logger.c        # All logging functionality
│   │   └── logger.h
│   ├── network/            # Network communication
│   │   ├── websocket.c     # WebSocket connection handling
│   │   ├── websocket.h
│   │   ├── okx_parser.c    # OKX JSON message parsing
│   │   └── okx_parser.h
│   ├── compute/            # Computational algorithms
│   │   ├── vwap_calculator.c # VWAP computation worker
│   │   ├── vwap_calculator.h
│   │   ├── correlation.c   # Correlation analysis worker
│   │   └── correlation.h
│   └── scheduler/          # Scheduling and timing
│       ├── scheduler.c     # Precise timing coordinator
│       └── scheduler.h
├── include/
│   └── common.h           # Common definitions and includes
├── Makefile              # Build system
└── README.md            # This file
```

## Key Features

- **Multi-threaded Architecture**: Separate threads for WebSocket handling, trade processing, scheduling, and computations
- **Efficient Data Management**: Circular buffers and sliding windows for optimal memory usage
- **Real-time Analytics**: 
  - 15-minute VWAP calculations updated every minute
  - Cross-asset Pearson correlations with lag analysis
- **Precise Scheduling**: Drift-compensating scheduling using `clock_nanosleep`
- **Comprehensive Logging**: Raw trades, computed metrics, and system performance data
- **Graceful Shutdown**: Clean resource cleanup on SIGINT/SIGTERM

## Module Overview

### Core Components

- **main.c**: Entry point, thread management, and resource coordination
- **config.h**: Global configuration and symbol definitions

### Utils Module
- **time_utils**: Time conversion and formatting utilities
- **system_monitor**: CPU and memory usage monitoring

### Data Module
- **structures.h**: Core data structure definitions
- **queue**: Thread-safe circular queue for raw trade messages
- **sliding_window**: Efficient sliding window with O(1) VWAP calculation
- **vwap_history**: Circular buffer for historical VWAP data

### Logging Module
- **logger**: All file I/O operations and performance logging

### Network Module
- **websocket**: WebSocket connection management and reconnection logic
- **okx_parser**: OKX-specific JSON message parsing

### Compute Module
- **vwap_calculator**: Worker thread for VWAP calculations
- **correlation**: Pearson correlation analysis with lag detection

### Scheduler Module
- **scheduler**: Precise timing coordinator with drift compensation

## Building and Running

### Prerequisites

```bash
# Install dependencies (Ubuntu/Debian)
make deps
```

### Build Options

```bash
# Default build
make

# Debug build with symbols
make debug

# Optimized release build
make release

# Clean build artifacts
make clean

# Clean everything including data files
make cleanall
```

### Running

```bash
# Run directly
make run

# Run in background
make background

# Or build and run manually
make
./main
```

### Stopping

```bash
# Send graceful shutdown signal
kill -SIGINT <PID>

# Or use Ctrl+C if running in foreground
```

## Monitored Symbols

The application monitors 8 cryptocurrency pairs:
- BTC-USDT, ADA-USDT, ETH-USDT
- DOGE-USDT, XRP-USDT, SOL-USDT  
- LTC-USDT, BNB-USDT

## Output Data

All data is stored in the `data/` directory:

- `data/trades/`: Raw trade messages (JSONL format)
- `data/metrics/vwap/`: Per-minute VWAP values (CSV)
- `data/metrics/correlations/`: Cross-asset correlations (CSV)
- `data/performance/`: System performance metrics (CSV)

## Performance Characteristics

- **Low Latency**: Minimal processing overhead with efficient data structures
- **Memory Efficient**: Bounded memory usage with circular buffers
- **CPU Optimized**: O(1) VWAP calculations, efficient correlation analysis
- **Reliable**: Automatic reconnection, comprehensive error handling

## Configuration

Key parameters can be adjusted in `include/common.h`:
- `WINDOW_MINUTES`: Sliding window duration (default: 15 minutes)
- `WINDOW_CAPACITY`: Maximum trades per symbol (default: 50,000)
- `MOVING_AVG_POINTS`: Correlation analysis window (default: 8 points)
- `MAX_LAG_MINUTES`: Maximum correlation lag (default: 60 minutes)

## Architecture Benefits

The modular design provides:

1. **Maintainability**: Clear separation of concerns
2. **Testability**: Individual modules can be unit tested
3. **Reusability**: Components can be reused in other projects
4. **Scalability**: Easy to add new features or modify existing ones
5. **Readability**: Well-organized code structure

## Author

Fraidakis Ioannis - September 2025