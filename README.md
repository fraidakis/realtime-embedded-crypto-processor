# Real-Time Cryptocurrency Transaction Processing System

A high-performance, real-time cryptocurrency trade data processor designed for Raspberry Pi. This application connects to the OKX WebSocket API to receive live trade data, processes it using a multi-threaded architecture, and performs real-time analytics including VWAP calculations and correlation analysis with precise minute-boundary scheduling.

## Project Overview

This system demonstrates sophisticated financial data processing on resource-constrained embedded platforms, implementing a complete real-time transaction processing pipeline that maintains sub-millisecond scheduling precision while processing continuous cryptocurrency trade streams from the OKX exchange.

### Key Capabilities

- **Real-time Processing**: Continuous ingestion and processing of live cryptocurrency trade data
- **Precise Scheduling**: Sub-millisecond drift scheduling using `clock_nanosleep` with dynamic deadline compensation
- **Financial Analytics**: 15-minute VWAP calculations and cross-asset Pearson correlation analysis
- **Resource Efficiency**: Optimized for Raspberry Pi Zero W (512MB RAM, single core)
- **Production Quality**: Comprehensive error handling, automatic reconnection, and graceful shutdown

## Project Structure

The project employs a modular architecture with clear separation of concerns:

```
├── src/
│   ├── main.c              # Main entry point and thread coordination
│   ├── config.h            # Configuration constants and global symbols
│   ├── utils/              # Utility functions
│   │   ├── time_utils.c    # Time conversion and formatting utilities
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
├── report/                # Technical documentation
│   ├── report.tex         # Comprehensive technical report
│   └── plots/             # Performance analysis plots
├── data/                  # Output data directory (created at runtime)
│   ├── trades/            # Raw trade logs (JSONL format)
│   ├── metrics/           # Computed analytics
│   │   ├── vwap/          # VWAP calculations (CSV)
│   │   └── correlations/  # Correlation analysis (CSV)
│   └── performance/       # System metrics (CSV)
├── Makefile              # Build system
└── README.md            # This file
```

## System Architecture

### Multi-threaded Design

The system implements a sophisticated 5-thread architecture optimized for real-time performance:

1. **WebSocket Thread (Producer)**: Minimal processing WebSocket handler using libwebsockets
2. **Trade Processor Thread (Consumer)**: JSON parsing, logging, and sliding window updates
3. **Scheduler Thread (Coordinator)**: Precise minute-boundary timing with drift compensation
4. **VWAP Worker Thread**: Volume-weighted average price calculations
5. **Correlation Worker Thread**: Cross-asset Pearson correlation analysis

### Data Flow Pipeline

```
OKX WebSocket → Raw Queue → JSON Parser → Sliding Windows → Analytics → File Output
     ↓              ↓           ↓            ↓             ↓         ↓
 Timestamp      Thread-Safe   Custom      O(1) VWAP    Real-time  Performance
   Capture       Buffering    Parser      Updates      Metrics     Logging
```

### Synchronization Mechanisms

- **Mutexes**: Protect shared data structures from race conditions
- **Condition Variables**: Efficient blocking for empty queue conditions
- **Barriers**: Coordinate scheduler and worker thread execution

## Technical Innovations

### Performance Optimizations

- **Custom JSON Parser**: 30% faster than cJSON through zero-allocation design
- **Circular Buffer Architecture**: O(1) operations with bounded memory usage
- **Concurrent I/O Hiding**: 15% reduction in scheduling drift through I/O overlap
- **Dynamic Deadline Compensation**: Exponential moving average for drift-free scheduling

### Memory Management

- **Pre-allocated Structures**: Eliminates runtime malloc/free operations
- **Memory Footprint**: 10.69 MB total allocation (2.1% of available RAM)
- **Leak Prevention**: Static allocation prevents memory fragmentation

### Real-time Scheduling

- **Clock Source**: `CLOCK_MONOTONIC` for immunity to system time adjustments
- **Absolute Timing**: `TIMER_ABSTIME` prevents drift accumulation
- **Compensation Algorithm**: EMA-based execution time prediction

## Monitored Cryptocurrency Pairs

The system processes real-time data for 8 major trading pairs:
- **BTC-USDT** (Bitcoin)
- **ETH-USDT** (Ethereum)  
- **ADA-USDT** (Cardano)
- **DOGE-USDT** (Dogecoin)
- **XRP-USDT** (Ripple)
- **SOL-USDT** (Solana)
- **LTC-USDT** (Litecoin)
- **BNB-USDT** (Binance Coin)

## Computational Tasks

### Task 1: Real-time Trade Logging
- **Objective**: Log every incoming trade immediately to persistent storage
- **Performance**: Sub-millisecond processing latency
- **Output**: Raw JSON trade data in `data/trades/<SYMBOL>.jsonl`

### Task 2: VWAP Calculation (Per-Minute)
- **Objective**: Compute 15-minute volume-weighted average price for each symbol
- **Algorithm**: O(1) calculation using running sums in sliding windows
- **Output**: Minute-level VWAP and volume data in `data/metrics/vwap/<SYMBOL>.csv`

### Task 3: Correlation Analysis (Per-Minute)
- **Objective**: Calculate Pearson correlations between symbols with lag analysis
- **Method**: 8-point correlation windows with up to 60-minute lag detection
- **Output**: Correlation coefficients and lag times in `data/metrics/correlations/<SYMBOL>.csv`

## Building and Running

### Prerequisites

Install required dependencies on Ubuntu/Debian:

```bash
# Install libwebsockets and build tools manually
sudo apt-get update
sudo apt-get install libwebsockets-dev build-essential

# For ARM cross-compilation (optional)
sudo apt-get install gcc-arm-linux-gnueabihf
```

### Build Options

```bash
# Standard build
make

# ARM cross-compilation for Raspberry Pi
make arm

# Clean build artifacts
make clean

# Clean ARM build artifacts  
make clean-arm

# Clean everything including generated data
make clean-all
```

### Execution

```bash
# Build and run directly
make run

# Run in background with output redirection
make background

# Manual execution
make
./main
```

### Process Management

```bash
# Kill running instances cleanly
make kill

# Or manually with SIGTERM for graceful shutdown
kill -TERM <PID>

# Or use Ctrl+C if running in foreground
```
## Output Data Structure

All generated data is organized in the `data/` directory:

### Trade Data
- **Location**: `data/trades/`
- **Format**: JSONL (JSON Lines)
- **Content**: Raw OKX WebSocket messages with metadata
- **Example**: `{"arg":{"channel":"trades","instId":"BTC-USDT"},"data":[...]}`

### Analytics Data
- **VWAP Results**: `data/metrics/vwap/<SYMBOL>.csv`
  - Format: `timestamp_iso,vwap,volume`
  - Updated every minute with 15-minute window calculations

- **Correlation Results**: `data/metrics/correlations/<SYMBOL>.csv`
  - Format: `timestamp_iso,correlated_with,correlation,lag_timestamp_iso`
  - Cross-asset correlation analysis with optimal lag detection

### Performance Metrics
- **System Resources**: `data/performance/system.csv`
  - CPU utilization and memory usage monitoring
  - Format: `timestamp_ms,cpu_percent,memory_mb`

- **Scheduling Precision**: `data/performance/scheduler.csv`
  - Drift analysis for real-time scheduling validation
  - Format: `scheduled_ms,actual_ms,drift_ms`

- **Processing Latency**: `data/performance/latency.csv`
  - End-to-end latency breakdown per trade
  - Format: `symbol_idx,exchange_ts,recv_ts,process_ts,network_lat,process_lat,total_lat`

## Performance Characteristics

Based on 62-hour continuous operation testing:

### Timing Precision
- **Mean Scheduling Drift**: 0.15 ms from minute boundaries
- **Distribution**: Near-Gaussian with σ = 1.58 ms
- **Maximum Deviation**: +10.41 ms / -4.99 ms

### Resource Efficiency
- **CPU Utilization**: 98.8% idle time (1.2% average load)
- **Memory Usage**: 11.8 MB stable footprint (2.3% of available RAM)
- **Processing Latency**: 2.2 ms average (excluding network latency)

### Throughput Scalability
- **Peak Performance**: 114 messages/second processed successfully
- **Linear Scaling**: 0.98 correlation between CPU usage and message rate
- **Theoretical Capacity**: >1,600 messages/second before saturation

## Configuration Parameters

Key system parameters can be adjusted in `include/common.h`:

```c
#define WINDOW_MINUTES 15           // VWAP sliding window duration
#define WINDOW_CAPACITY 50000       // Maximum trades per symbol buffer
#define MOVING_AVG_POINTS 8         // Correlation analysis window size
#define MAX_LAG_MINUTES 60          // Maximum correlation lag detection
#define FSYNC_PER_WRITE 0           // Durability vs performance trade-off
```

## Architecture Benefits

The modular design provides several key advantages:

1. **Maintainability**: Clear separation of concerns enables independent module development
2. **Testability**: Individual components can be unit tested in isolation
3. **Reusability**: Core modules can be adapted for other financial data processing applications
4. **Scalability**: Easy addition of new trading pairs or analytical algorithms
5. **Performance**: Optimized data structures and algorithms for embedded constraints

## Technical Documentation

For comprehensive technical details, including mathematical foundations, performance analysis, and implementation specifics, refer to the complete technical report: `report/report.tex`

## Author

**Fraidakis Ioannis**  
Department of Electrical and Computer Engineering  
Aristotle University of Thessaloniki  
September 2025

---

*This project demonstrates the feasibility of implementing sophisticated real-time financial data processing systems on resource-constrained embedded platforms through careful architectural design and algorithmic optimization.*