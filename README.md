# Real-Time Cryptocurrency Trade Processor

This project is a high-performance, real-time cryptocurrency trade data processor, designed to run efficiently on resource-constrained devices like the Raspberry Pi. It connects to the OKX WebSocket API, processes live trade data using a multi-threaded architecture, and performs real-time analytics, including VWAP calculations and correlation analysis with sub-millisecond precision.

## Key Features

- **Real-Time Data Processing**: Ingests and processes live cryptocurrency trade data from the OKX exchange.
- **High-Precision Scheduling**: Utilizes `clock_nanosleep` for sub-millisecond scheduling accuracy.
- **In-Depth Financial Analytics**: Calculates 15-minute Volume-Weighted Average Prices (VWAP) and cross-asset Pearson correlations.
- **Resource-Efficient**: Optimized for low-power devices such as the Raspberry Pi Zero W.
- **Robust and Reliable**: Features comprehensive error handling, automatic reconnection, and graceful shutdown capabilities.

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



## Visualizations

The system includes a powerful Python-based visualization tool that generates detailed performance and analysis plots. Here are some examples from a 62-hour continuous run:

### Scheduler Timing Drift
![Scheduler Timing Drift](scheduler_timing_drift_performance.png)

### Network and Processing Latency
![Network and Processing Latency](plots/62-hours/network_processing_latency_breakdown.png)

### System Resource Usage
![System Resource Usage](plots/62-hours/system_resources_cpu_memory_usage.png)

### CPU Load vs. Message Throughput
![CPU Load vs. Message Throughput](plots/62-hours/cpu_load_vs_message_throughput.png)

## Getting Started

Follow these instructions to build the application, run it, and generate your own visualizations.

### Prerequisites

Ensure you have the following dependencies installed on your Ubuntu/Debian-based system:

```bash
sudo apt-get update
sudo apt-get install -y libwebsockets-dev build-essential python3 python3-pip
pip3 install pandas matplotlib seaborn
```
### Building the Application

You can build the application using the provided `Makefile`:

- **Standard Build**: `make`
- **ARM Cross-Compilation**: `make arm`
- **Clean Build Artifacts**: `make clean`

### Running the Application

To start the data processor, run:
```bash
make run
```
Alternatively, you can run it in the background:
```bash
make background
```
The application will begin collecting and processing data, storing it in the `data/` directory.

### Generating Visualizations

Once you have collected some data, you can generate the analysis plots using the `plot.py` script:

```bash
python3 plot.py
```
The generated plots will be saved in the `plots/` directory.

## System Architecture

The application employs a 5-thread architecture for optimal performance:

1.  **WebSocket Thread**: Handles the connection to the OKX WebSocket API.
2.  **Trade Processor Thread**: Parses incoming JSON messages and updates data structures.
3.  **Scheduler Thread**: Coordinates the execution of analytics tasks with high precision.
4.  **VWAP Worker Thread**: Calculates the Volume-Weighted Average Price.
5.  **Correlation Worker Thread**: Computes the Pearson correlation between assets.

For more in-depth technical details, please refer to the original `README.md` content, which has been preserved in `README_TECHNICAL.md`.

### Data Flow Pipeline

```
OKX WebSocket → Raw Queue → JSON Parser → Sliding Windows → Analytics → File Output
     ↓              ↓            ↓              ↓               ↓           ↓
 Timestamp     Thread-Safe     Custom       O(1) VWAP       Real-time  Performance
  Capture       Buffering      Parser        Updates         Metrics     Logging
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

## Technical Documentation

For comprehensive technical details, including mathematical foundations, performance analysis, and implementation specifics, refer to the complete technical report: `report/report.tex`


## Author

**Fraidakis Ioannis**
*Department of Electrical and Computer Engineering*
*Aristotle University of Thessaloniki*
*September 2025*