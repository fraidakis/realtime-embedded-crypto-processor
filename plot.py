#!/usr/bin/env python3
"""
OKX Trading System Metrics Visualization and Analysis Script

This comprehensive script analyzes and visualizes performance metrics from the 
real-time cryptocurrency trading system, providing insights into:
- Scheduler timing accuracy and drift patterns
- Network and processing latency breakdown
- System resource utilization (CPU and memory)
- Message processing throughput and correlation with system load
- Cross-cryptocurrency price correlation analysis
"""

import json
from pathlib import Path
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

# Configure global plotting style for professional appearance
plt.style.use('seaborn-v0_8')
sns.set_palette("husl")

class CryptocurrencyTradingSystemVisualizer:
    """
    Comprehensive visualization and analysis tool for cryptocurrency trading system metrics.
    
    This class provides methods to load, process, and visualize various performance metrics
    including scheduler drift, latency components, system resource usage, and correlation
    analysis for real-time cryptocurrency trading systems.
    
    Attributes:
        data_directory (Path): Directory containing the performance data files
        supported_trading_symbols (list): List of cryptocurrency trading pairs to analyze
        visualization_colors (dict): Color palette for consistent chart styling
        style_configuration (dict): Typography and layout parameters
        output_directory (Path): Directory for saving generated visualizations
    """
    
    def __init__(self, data_directory='data'):
        """
        Initialize the trading system metrics visualizer.
        
        Args:
            data_directory (str): Path to directory containing performance data files
        """
        self.data_directory = Path(data_directory)
        
        # Define cryptocurrency trading pairs to analyze
        self.supported_trading_symbols = [
            "BTC-USDT", "ADA-USDT", "ETH-USDT", "DOGE-USDT",
            "XRP-USDT", "SOL-USDT", "LTC-USDT", "BNB-USDT"
        ]

        # Professional color palette for consistent chart styling
        self.visualization_colors = {
            'primary': '#1f77b4',      # Professional blue for main data series
            'secondary': '#ff7f0e',    # Warm orange for secondary elements
            'accent': '#2ca02c',       # Green for highlights and success indicators
            'background': '#f8f9fa',   # Light gray for chart backgrounds
            'text': '#2c3e50',         # Dark blue-gray for readable text
            'highlight': '#e74c3c'     # Red for important highlights
        }

        # Typography and layout configuration for consistent styling
        self.style_configuration = {
            'font_family': 'DejaVu Sans',
            'base_font_size': 14,
            'title_font_size': 20,
            'axis_label_size': 16,
            'tick_label_size': 12,
            'legend_font_size': 12,
            'image_dpi': 150,
            'large_figure_size': (18, 12),
            'medium_figure_size': (16, 10),
            'small_figure_size': (12, 8),
            'line_alpha': 0.8,
            'fill_alpha': 0.7,
            'grid_alpha': 0.3,
            'text_box_alpha': 0.95
        }

        # Create output directory for generated visualizations
        images_directory_name = self._generate_output_directory_name()
        self.output_directory = self.data_directory.parent.parent.parent / images_directory_name
        self.output_directory.mkdir(parents=True, exist_ok=True)
        
    def _generate_output_directory_name(self):
        """
        Generate appropriate output directory name based on data directory.

        Returns:
            str: Output directory name for storing generated visualizations
        """
        path_parts = self.data_directory.parts
        
        if len(path_parts) >= 2 and path_parts[-1] == 'data':
            parent_dir = path_parts[-2]
            return f'results/{parent_dir}'
        else:
            return 'results'    
        
    def _setup_plot_style(self):
        """
        Set up consistent plotting style and parameters for all visualizations.
        """
        sns.set_style('whitegrid')
        plt.rcParams['font.family'] = self.style_configuration['font_family']
        plt.rcParams['font.size'] = self.style_configuration['base_font_size']
        plt.rcParams['axes.labelsize'] = self.style_configuration['axis_label_size']
        plt.rcParams['axes.titlesize'] = self.style_configuration['title_font_size']
        plt.rcParams['xtick.labelsize'] = self.style_configuration['tick_label_size']
        plt.rcParams['ytick.labelsize'] = self.style_configuration['tick_label_size']
        plt.rcParams['figure.facecolor'] = '#ffffff'  # White background for figures

    def _configure_axis(self, axis, title, xlabel, ylabel, legend_loc='upper right'):
        """
        Configure axis labels, title, legend, grid, and ticks.
        """
        axis.set_ylabel(ylabel, fontsize=self.style_configuration['axis_label_size'], color=self.visualization_colors['text'], labelpad=10)
        axis.set_xlabel(xlabel, fontsize=self.style_configuration['axis_label_size'], color=self.visualization_colors['text'], labelpad=10)
        axis.set_title(title, fontsize=self.style_configuration['title_font_size'], fontweight='bold', color=self.visualization_colors['text'], pad=20)
        axis.legend(fontsize=self.style_configuration['legend_font_size'], loc=legend_loc, frameon=True, fancybox=True, shadow=True, facecolor='white', edgecolor='gray', framealpha=0.9)
        axis.grid(True, alpha=self.style_configuration['grid_alpha'], linestyle='--')
        axis.set_axisbelow(True)
        axis.set_facecolor(self.visualization_colors['background'])
        axis.tick_params(axis='both', labelsize=self.style_configuration['tick_label_size'])

    def _add_stats_text(self, axis, stats_text, position=(0.02, 0.98)):
        """
        Add statistics text box to the axis.
        """
        axis.text(
            position[0], position[1], stats_text,
            transform=axis.transAxes,
            verticalalignment='top', horizontalalignment='left',
            bbox=dict(
                boxstyle='round,pad=0.8',
                facecolor=self.visualization_colors['background'],
                alpha=self.style_configuration['text_box_alpha'],
                edgecolor=self.visualization_colors['primary'], linewidth=1.5
            ),
            fontsize=self.style_configuration['tick_label_size'],
            family='monospace', color=self.visualization_colors['text'],
            linespacing=1.4
        )

    def _save_plot(self, filename):
        """
        Save the plot to file and close.
        """
        plt.tight_layout()
        plt.savefig(
            self.output_directory / filename,
            dpi=self.style_configuration['image_dpi'], bbox_inches='tight'
        )
        plt.close()

    def load_scheduler_performance_data(self):
        """
        Load and preprocess scheduler performance metrics from CSV file.
        
        Returns:
            pandas.DataFrame or None: DataFrame containing scheduler timing data with
                                    computed datetime columns, or None if file not found
        """
        try:
            scheduler_file_path = self.data_directory / 'performance' / 'scheduler.csv'
            if not scheduler_file_path.exists():
                print(f"Warning: Scheduler data file not found at {scheduler_file_path}")
                return None
                
            scheduler_dataframe = pd.read_csv(scheduler_file_path)
            
            # Convert millisecond timestamps to datetime objects for analysis
            scheduler_dataframe['scheduled_datetime'] = pd.to_datetime(
                scheduler_dataframe['scheduled_ms'], unit='ms'
            )
            
            return scheduler_dataframe
            
        except Exception as error:
            print(f"Error loading scheduler performance data: {error}")
            return None
    
    def load_latency_performance_data(self):
        """
        Load and preprocess network and processing latency metrics from CSV file.
        
        Returns:
            pandas.DataFrame or None: DataFrame containing latency measurements with
                                    symbol mapping and datetime conversion, or None if file not found
        """
        try:
            latency_file_path = self.data_directory / 'performance' / 'latency.csv'
            if not latency_file_path.exists():
                print(f"Warning: Latency data file not found at {latency_file_path}")
                return None
                
            latency_dataframe = pd.read_csv(latency_file_path)
            
            # Convert exchange timestamp to datetime for time-series analysis
            latency_dataframe['exchange_datetime'] = pd.to_datetime(
                latency_dataframe['exchange_ts_ms'], unit='ms'
            )
            
            # Map symbol indices to readable cryptocurrency pair names
            latency_dataframe['trading_symbol'] = latency_dataframe['symbol_index'].map(
                lambda index: (self.supported_trading_symbols[index] 
                             if index < len(self.supported_trading_symbols) 
                             else f"Symbol_{index}")
            )
            
            return latency_dataframe
            
        except Exception as error:
            print(f"Error loading latency performance data: {error}")
            return None
    
    def load_system_resource_data(self):
        """
        Load and preprocess system resource utilization metrics from CSV file.
        
        Returns:
            pandas.DataFrame or None: DataFrame containing CPU and memory usage data
                                    with computed idle time, or None if file not found
        """
        try:
            system_file_path = self.data_directory / 'performance' / 'system.csv'
            if not system_file_path.exists():
                print(f"Warning: System resource data file not found at {system_file_path}")
                return None
                
            system_dataframe = pd.read_csv(system_file_path)
            
            # Convert timestamp to datetime for time-series analysis
            system_dataframe['timestamp_datetime'] = pd.to_datetime(
                system_dataframe['timestamp_ms'], unit='ms'
            )
            
            # Calculate CPU idle percentage from usage percentage
            system_dataframe['cpu_idle_percent'] = 100 - system_dataframe['cpu_percent']
            
            return system_dataframe
            
        except Exception as error:
            print(f"Error loading system resource data: {error}")
            return None
    
    def load_trading_message_logs(self):
        """
        Load and process trading message logs to calculate message processing throughput.
        
        Returns:
            pandas.DataFrame or None: DataFrame containing processed trade messages with
                                    timestamps and trading details, or None if no data found
        """
        trading_directory = self.data_directory / 'trades'
        if not trading_directory.exists():
            print(f"Warning: Trading logs directory not found at {trading_directory}")
            return None
            
        all_trading_messages = []
        
        # Process JSONL files for each supported trading symbol
        for trading_symbol in self.supported_trading_symbols:
            try:
                trade_log_file = trading_directory / f'{trading_symbol}.jsonl'
                if trade_log_file.exists():
                    with open(trade_log_file, 'r') as file_handle:
                        for line_content in file_handle:
                            try:
                                trade_message = json.loads(line_content.strip())
                                
                                # Extract trade data if present and valid
                                if 'data' in trade_message and trade_message['data']:
                                    trade_details = trade_message['data'][0]
                                    all_trading_messages.append({
                                        'trading_symbol': trading_symbol,
                                        'timestamp_ms': int(trade_details['ts']),
                                        'price': float(trade_details['px']),
                                        'trade_size': float(trade_details['sz'])
                                    })
                                    
                            except (json.JSONDecodeError, KeyError, ValueError):
                                # Skip malformed JSON lines or missing required fields
                                continue
                                
            except Exception as error:
                print(f"Error loading trading logs for {trading_symbol}: {error}")
        
        if not all_trading_messages:
            print("No valid trading messages found in log files")
            return None
            
        # Convert to DataFrame and add datetime column for analysis
        trading_dataframe = pd.DataFrame(all_trading_messages)
        trading_dataframe['timestamp_datetime'] = pd.to_datetime(
            trading_dataframe['timestamp_ms'], unit='ms'
        )
        
        return trading_dataframe
    
    def visualize_scheduler_drift_patterns(self):
        """
        Create comprehensive visualization of scheduler timing drift patterns.

        Generates a dual-panel plot showing:
        - Time series of scheduler drift over the entire monitoring period
        - Statistical distribution of drift values with kernel density estimation

        The visualization helps identify systematic timing issues and drift patterns
        in the real-time trading system scheduler.
        """
        scheduler_performance_data = self.load_scheduler_performance_data()
        if scheduler_performance_data is None or len(scheduler_performance_data) == 0:
            print("No scheduler performance data available for visualization")
            return

        # Set up consistent plotting style
        self._setup_plot_style()

        # Create figure with two vertically stacked subplots
        figure, (time_series_axis, distribution_axis) = plt.subplots(
            2, 1, figsize=(18, 12), constrained_layout=True
        )

        # Extract drift data and time information for analysis
        scheduler_drift_values = scheduler_performance_data['drift_ms']
        scheduled_timestamps = scheduler_performance_data['scheduled_datetime']

        # Convert datetime to hours from monitoring start for consistent x-axis
        monitoring_start_time = scheduled_timestamps.min()
        hours_elapsed_from_start = (
            scheduled_timestamps - monitoring_start_time
        ).dt.total_seconds() / 3600

        # Plot time series of scheduler drift
        time_series_axis.plot(
            hours_elapsed_from_start, scheduler_drift_values,
            linewidth=1, color=self.visualization_colors['primary'], alpha=self.style_configuration['line_alpha'], label='Measured Drift'
        )
        time_series_axis.axhline(
            y=0, color=self.visualization_colors['highlight'], linestyle='--', linewidth=2,
            alpha=self.style_configuration['line_alpha'], label='Perfect Timing Reference'
        )

        # Configure time series subplot
        time_series_axis.set_ylabel('Scheduler Drift (ms)', fontsize=self.style_configuration['axis_label_size'], color=self.visualization_colors['text'], labelpad=10)
        time_series_axis.set_xlabel('Monitoring Time (hours)', fontsize=self.style_configuration['axis_label_size'], color=self.visualization_colors['text'], labelpad=10)
        time_series_axis.set_title('Scheduler Drift Time Series', 
                                  fontsize=self.style_configuration['title_font_size'], 
                                  fontweight='bold', 
                                  color=self.visualization_colors['text'], 
                                  pad=20)
        time_series_axis.legend(fontsize=self.style_configuration['legend_font_size'], loc='upper right', frameon=True, fancybox=True, shadow=True, facecolor='white', edgecolor='gray', framealpha=0.9)
        time_series_axis.tick_params(axis='both')
        time_series_axis.minorticks_on()
        time_series_axis.grid(True, alpha=self.style_configuration['grid_alpha'], linestyle='--')
        time_series_axis.set_facecolor(self.visualization_colors['background'])

        # Configure x-axis with 4-hour intervals for readability
        maximum_monitoring_hours = hours_elapsed_from_start.max()
        hour_tick_positions = np.arange(0, maximum_monitoring_hours + 4, 4)
        time_series_axis.set_xticks(hour_tick_positions)
        time_series_axis.set_xticklabels([f'{int(h)}' for h in hour_tick_positions])

        # Create distribution histogram with kernel density estimation
        sns.histplot(
            data=scheduler_performance_data, x='drift_ms', kde=True, ax=distribution_axis,
            color=self.visualization_colors['secondary'], alpha=self.style_configuration['fill_alpha'], edgecolor='white', linewidth=0.5, bins=50
        )
        distribution_axis.axvline(
            x=0, color=self.visualization_colors['highlight'], linestyle='--', linewidth=2,
            alpha=self.style_configuration['line_alpha'], label='Perfect Timing Reference'
        )

        # Configure distribution subplot
        distribution_axis.set_xlabel('Scheduler Drift (ms)', fontsize=self.style_configuration['axis_label_size'], color=self.visualization_colors['text'], labelpad=5)
        distribution_axis.set_ylabel('Frequency / Density', fontsize=self.style_configuration['axis_label_size'], color=self.visualization_colors['text'], labelpad=5)
        distribution_axis.set_title('Statistical Distribution of Scheduler Drift (with KDE)', fontsize=self.style_configuration['title_font_size'], fontweight='bold', color=self.visualization_colors['text'], pad=20)
        distribution_axis.legend(fontsize=self.style_configuration['legend_font_size'], loc='upper right', frameon=True, fancybox=True, shadow=True, facecolor='white', edgecolor='gray', framealpha=0.9)
        distribution_axis.tick_params(axis='both', labelsize=self.style_configuration['tick_label_size'])
        distribution_axis.minorticks_on()
        distribution_axis.grid(True, alpha=self.style_configuration['grid_alpha'], linestyle='--')
        distribution_axis.set_axisbelow(True)
        distribution_axis.set_facecolor(self.visualization_colors['background'])

        # Add comprehensive statistics summary box
        drift_statistics_summary = (
            "  Scheduler Drift Stats\n"
            "─────────────────────────\n"
            f"Minimum Drift : {scheduler_drift_values.min():5.2f} ms\n"
            f"Mean Drift    : {scheduler_drift_values.mean():5.2f} ms\n"
            f"Maximum Drift : {scheduler_drift_values.max():5.2f} ms\n"
            f"Standard Dev. : {scheduler_drift_values.std():5.2f} ms\n"
        )

        self._add_stats_text(distribution_axis, drift_statistics_summary, position=(0.04, 0.9))

        # Save high-quality visualization
        self._save_plot('scheduler_timing_drift_performance.png')

    def visualize_latency_component_analysis(self):
        """
        Create detailed visualization of network and processing latency components.

        Generates a stacked bar chart showing the breakdown of total latency into:
        - Network latency (time for message transmission)
        - Processing latency (time for computational processing)

        Data is aggregated by hour and outliers are filtered for cleaner visualization.
        Includes comprehensive performance statistics and trend analysis.
        """
        latency_performance_data = self.load_latency_performance_data()
        if latency_performance_data is None or len(latency_performance_data) == 0:
            print("No latency performance data available for visualization")
            return

        # Calculate total latency for outlier detection and filtering
        latency_performance_data['total_latency_ms'] = (
            latency_performance_data['network_latency_ms'] +
            latency_performance_data['processing_latency_ms']
        )

        # Remove extreme outliers (top 0.1%) to improve visualization clarity
        outlier_exclusion_threshold = latency_performance_data['total_latency_ms'].quantile(0.999)
        original_sample_count = len(latency_performance_data)
        filtered_latency_data = latency_performance_data[
            latency_performance_data['total_latency_ms'] <= outlier_exclusion_threshold
        ]
        filtered_sample_count = len(filtered_latency_data)

        print(f"   Filtered {original_sample_count - filtered_sample_count} extreme outliers "
              f"(top 0.1%) from {original_sample_count:,} data points")

        # Aggregate latency measurements by hour for trend analysis
        filtered_latency_data = filtered_latency_data.copy()
        filtered_latency_data['hourly_timestamp'] = filtered_latency_data['exchange_datetime'].dt.floor('H')
        hourly_latency_aggregation = filtered_latency_data.groupby('hourly_timestamp').agg({
            'network_latency_ms': 'mean',
            'processing_latency_ms': 'mean'
        }).reset_index()

        # Convert datetime to hours from monitoring start for consistent x-axis
        monitoring_start_time = hourly_latency_aggregation['hourly_timestamp'].min()
        hourly_latency_aggregation['hours_from_start'] = (
            hourly_latency_aggregation['hourly_timestamp'] - monitoring_start_time
        ).dt.total_seconds() / 3600

        # Set up consistent plotting style
        self._setup_plot_style()

        # Create large figure for detailed latency analysis
        figure, latency_axis = plt.subplots(1, 1, figsize=(20, 15))

        # Prepare data arrays for stacked bar visualization
        x_axis_positions = np.arange(len(hourly_latency_aggregation))
        network_latency_values = hourly_latency_aggregation['network_latency_ms'].values
        processing_latency_values = hourly_latency_aggregation['processing_latency_ms'].values
        total_latency_values = network_latency_values + processing_latency_values

        # Use consistent color palette for component identification
        network_component_color = self.visualization_colors['primary']
        processing_component_color = self.visualization_colors['secondary']
        total_latency_color = self.visualization_colors['accent']

        # Create stacked bar chart showing latency component breakdown
        network_bars = latency_axis.bar(
            x_axis_positions, network_latency_values,
            label='Network Transmission Latency',
            alpha=self.style_configuration['fill_alpha'],
            color=network_component_color,
            width=0.8, edgecolor='white', linewidth=0.5
        )

        processing_bars = latency_axis.bar(
            x_axis_positions, processing_latency_values,
            bottom=network_latency_values,
            label='Computational Processing Latency',
            alpha=self.style_configuration['fill_alpha'],
            color=processing_component_color,
            width=0.8, edgecolor='white', linewidth=0.5
        )

        # Overlay total latency trend line for comprehensive analysis
        latency_axis.plot(
            x_axis_positions, total_latency_values,
            color=total_latency_color, linewidth=2.5, marker='o', markersize=4,
            label='Total System Latency',
            alpha=self.style_configuration['line_alpha'], linestyle='-'
        )

        # Configure axis using helper
        self._configure_axis(
            latency_axis,
            'Latency Component Analysis (Outliers Filtered)',
            'Monitoring Time (hours)',
            'Latency (milliseconds)',
            legend_loc='upper right'
        )


        # Enhance grid and background appearance (grid already in helper, but specify axis='y' if needed)
        latency_axis.grid(True, alpha=self.style_configuration['grid_alpha'], linestyle='--', axis='y')

        # Configure x-axis with hour-based time labels
        maximum_monitoring_hours = hourly_latency_aggregation['hours_from_start'].max()
        hour_tick_intervals = np.arange(0, maximum_monitoring_hours + 4, 4)
        valid_hour_positions = hour_tick_intervals[hour_tick_intervals <= len(x_axis_positions)]

        tick_positions = []
        tick_labels = []

        for hour_marker in valid_hour_positions:
            closest_position_index = np.argmin(
                np.abs(hourly_latency_aggregation['hours_from_start'] - hour_marker)
            )
            if closest_position_index < len(x_axis_positions):
                tick_positions.append(x_axis_positions[closest_position_index])
                tick_labels.append(f'{int(hour_marker)}')

        latency_axis.set_xticks(tick_positions)
        latency_axis.set_xticklabels(tick_labels)

        # Adjust plot boundaries for better visual balance
        latency_axis.set_xlim(-len(x_axis_positions) * 0.115, len(x_axis_positions) - 1 + len(x_axis_positions) * 0.05)

        # Format y-axis for improved readability
        latency_axis.yaxis.set_major_formatter(plt.FuncFormatter(lambda x, p: f'{x:.0f}'))

        # Add comprehensive performance metrics summary
        average_network_latency = np.mean(network_latency_values)
        average_processing_latency = np.mean(processing_latency_values)
        average_total_latency = np.mean(total_latency_values)
        maximum_total_latency = np.max(total_latency_values)
        minimum_total_latency = np.min(total_latency_values)

        performance_metrics_summary = (
            "    Latency Performance Metrics\n"
            "────────────────────────────────────\n"
            f"Network Latency (mean)   : {average_network_latency:6.1f} ms\n"
            f"Processing Latency (mean): {average_processing_latency:6.1f} ms\n"
            f"Total Latency (mean)     : {average_total_latency:6.1f} ms\n"
            f"Total Latency (minimum)  : {minimum_total_latency:6.1f} ms\n"
            f"Total Latency (maximum)  : {maximum_total_latency:6.1f} ms\n"
            f"Hourly Data Points       : {len(total_latency_values):3d} hours\n"
        )

        # Position comprehensive statistics in top-left corner
        self._add_stats_text(latency_axis, performance_metrics_summary, position=(0.015, 0.97))

        # Save high-quality visualization with optimized layout
        self._save_plot('network_processing_latency_breakdown.png')

    def visualize_cpu_memory_performance(self):
        """Create comprehensive time series visualization of CPU idle time and memory usage."""
        system_resource_data = self.load_system_resource_data()
        if system_resource_data is None or len(system_resource_data) == 0:
            print("No system resource data available for visualization")
            return

        # Set up consistent plotting style
        self._setup_plot_style()

        figure, (cpu_axis, memory_axis) = plt.subplots(2, 1, figsize=(18, 13), constrained_layout=True)

        # Extract CPU and memory data for visualization
        cpu_idle_percentage = system_resource_data['cpu_idle_percent']
        memory_usage_mb = system_resource_data['memory_mb']
        monitoring_timestamps = system_resource_data['timestamp_datetime']
        
        # Convert datetime to hours from monitoring start
        monitoring_start_time = monitoring_timestamps.min()
        hours_elapsed_from_start = (monitoring_timestamps - monitoring_start_time).dt.total_seconds() / 3600

        # CPU Idle percentage time series visualization
        cpu_visualization_color = self.visualization_colors['primary']
        cpu_axis.plot(hours_elapsed_from_start, cpu_idle_percentage, linewidth=1.5, color=cpu_visualization_color, alpha=self.style_configuration['line_alpha'], label='CPU Idle %')

        # Configure CPU axis
        self._configure_axis(
            cpu_axis,
            'CPU Idle Percentage Time Series',
            'Monitoring Time (hours)',
            'CPU Idle (%)',
            legend_loc='lower left'
        )
        cpu_axis.set_ylim(bottom=cpu_idle_percentage.min()*0.97, top=100)
        
        # Configure time axis with 4-hour intervals
        maximum_monitoring_hours = hours_elapsed_from_start.max()
        hour_tick_positions = np.arange(0, maximum_monitoring_hours + 4, 4)
        cpu_axis.set_xticks(hour_tick_positions)
        cpu_axis.set_xticklabels([f'{int(h)}' for h in hour_tick_positions])

        # Add CPU performance statistics summary
        cpu_performance_statistics = (
            " CPU Idle Performance Metrics\n"
            "──────────────────────────────\n"
            f"Minimum Idle : {cpu_idle_percentage.min():6.1f} %\n"
            f"Mean Idle    : {cpu_idle_percentage.mean():6.1f} %\n"
            f"Maximum Idle : {cpu_idle_percentage.max():6.1f} %\n"
        )

        self._add_stats_text(cpu_axis, cpu_performance_statistics, position=(0.76, 0.3))

        # Memory usage time series visualization
        memory_visualization_color = self.visualization_colors['secondary']
        memory_axis.plot(hours_elapsed_from_start, memory_usage_mb, linewidth=2, color=memory_visualization_color, alpha=self.style_configuration['line_alpha'], label='Memory Usage (MB)')

        # Configure memory axis
        self._configure_axis(
            memory_axis,
            'Memory Usage Time Series',
            'Monitoring Time (hours)',
            'Memory Usage (MB)',
            legend_loc='upper left'
        )
        
        # Configure time axis for memory plot
        memory_axis.set_xticks(hour_tick_positions)
        memory_axis.set_xticklabels([f'{int(h)}' for h in hour_tick_positions])

        # Add memory usage statistics summary
        memory_performance_statistics = (
            " Memory Usage Performance Metrics\n"
            "──────────────────────────────────\n"
            f"Minimum Usage : {memory_usage_mb.min():5.1f} MB\n"
            f"Maximum Usage : {memory_usage_mb.max():5.1f} MB\n"
        )

        self._add_stats_text(memory_axis, memory_performance_statistics, position=(0.74, 0.3))

        # Add subtle background coloring
        cpu_axis.set_facecolor(self.visualization_colors['background'])
        memory_axis.set_facecolor(self.visualization_colors['background'])

        self._save_plot('system_resources_cpu_memory_usage.png')

    def visualize_cpu_load_message_throughput_correlation(self):
        """
        Analyze and visualize correlation between CPU usage and message processing throughput.
        
        Creates dual time-series plots showing CPU usage percentage and message processing
        rate over time, allowing identification of potential relationships between system
        load and trading message throughput performance.
        """
        # Load required performance data sources
        system_resource_data = self.load_system_resource_data()
        trading_message_data = self.load_trading_message_logs()
        scheduler_performance_data = self.load_scheduler_performance_data()
        
        if (system_resource_data is None or len(system_resource_data) == 0 or
            trading_message_data is None or len(trading_message_data) == 0 or
            scheduler_performance_data is None or len(scheduler_performance_data) == 0):
            print("Insufficient data available for CPU vs message throughput correlation analysis")
            return

        # Aggregate data by 15-minute intervals for correlation analysis
        system_resource_data['time_interval'] = system_resource_data['timestamp_datetime'].dt.floor('15T')
        cpu_usage_aggregated = system_resource_data.groupby('time_interval')['cpu_percent'].mean().reset_index()

        trading_message_data['time_interval'] = trading_message_data['timestamp_datetime'].dt.floor('15T')
        message_throughput_aggregated = trading_message_data.groupby('time_interval').size().reset_index()
        message_throughput_aggregated.columns = ['time_interval', 'messages_per_interval']

        # Merge datasets on time interval for synchronized analysis
        synchronized_performance_data = pd.merge(
            cpu_usage_aggregated, message_throughput_aggregated, 
            on='time_interval', how='inner'
        )
        
        # Convert datetime to hours from monitoring start for consistent visualization
        monitoring_start_time = synchronized_performance_data['time_interval'].min()
        synchronized_performance_data['hours_from_start'] = (
            synchronized_performance_data['time_interval'] - monitoring_start_time
        ).dt.total_seconds() / 3600

        # Set up consistent plotting style
        self._setup_plot_style()

        figure, (cpu_usage_axis, message_rate_axis) = plt.subplots(2, 1, figsize=(20, 15), constrained_layout=True)

        # Calculate rolling means for trend smoothing
        smoothing_window_size = 10
        cpu_usage_rolling_mean = synchronized_performance_data['cpu_percent'].rolling(window=smoothing_window_size, center=True).mean()
        message_rate_rolling_mean = synchronized_performance_data['messages_per_interval'].rolling(window=smoothing_window_size, center=True).mean()

        # CPU Usage visualization subplot
        cpu_primary_color = self.visualization_colors['primary']
        trend_line_color = self.visualization_colors['secondary']

        cpu_usage_axis.plot(synchronized_performance_data['hours_from_start'], synchronized_performance_data['cpu_percent'], color=cpu_primary_color, linewidth=2, label='CPU Usage (%)', alpha=self.style_configuration['line_alpha'])
        cpu_usage_axis.plot(synchronized_performance_data['hours_from_start'], cpu_usage_rolling_mean, color=trend_line_color, linewidth=3, linestyle='--', alpha=0.9, label='Rolling Mean (10-pt)')
        cpu_usage_axis.set_xlabel('Monitoring Time (hours)', fontsize=self.style_configuration['axis_label_size'], color=self.visualization_colors['text'], labelpad=10)
        cpu_usage_axis.set_ylabel('CPU Usage (%)', fontsize=self.style_configuration['axis_label_size'], color=self.visualization_colors['text'], labelpad=20)
        cpu_usage_axis.set_ylim(0, synchronized_performance_data['cpu_percent'].max() * 1.05)
        cpu_usage_axis.legend(loc='upper left', fontsize=self.style_configuration['legend_font_size'], frameon=True, fancybox=True, shadow=True, facecolor='white', edgecolor='gray', framealpha=0.9)
        cpu_usage_axis.set_title('CPU Usage Percentage Over Time', fontsize=self.style_configuration['title_font_size'], fontweight='bold', color=self.visualization_colors['text'], pad=20)
        cpu_usage_axis.tick_params(axis='both', labelsize=self.style_configuration['tick_label_size'])
        cpu_usage_axis.grid(True, alpha=self.style_configuration['grid_alpha'], linestyle='--')
        cpu_usage_axis.set_axisbelow(True)
        cpu_usage_axis.set_facecolor(self.visualization_colors['background'])
        
        # Configure time axis with 4-hour intervals for CPU plot
        maximum_monitoring_hours = synchronized_performance_data['hours_from_start'].max()
        hour_tick_positions = np.arange(0, maximum_monitoring_hours + 4, 4)
        cpu_usage_axis.set_xticks(hour_tick_positions)
        cpu_usage_axis.set_xticklabels([f'{int(h)}' for h in hour_tick_positions])

        # Message Processing Rate visualization subplot
        message_primary_color = self.visualization_colors['primary']
        message_rate_axis.plot(synchronized_performance_data['hours_from_start'], synchronized_performance_data['messages_per_interval'], color=message_primary_color, linewidth=2, label='Messages per 15min', alpha=self.style_configuration['line_alpha'])
        message_rate_axis.plot(synchronized_performance_data['hours_from_start'], message_rate_rolling_mean, color=trend_line_color, linewidth=3, linestyle='--', alpha=0.9, label='Rolling Mean (10-pt)')
        message_rate_axis.set_xlabel('Monitoring Time (hours)', fontsize=self.style_configuration['axis_label_size'], color=self.visualization_colors['text'], labelpad=10)
        message_rate_axis.set_ylabel('Messages per 15min', fontsize=self.style_configuration['axis_label_size'], color=self.visualization_colors['text'], labelpad=5)
        message_rate_axis.set_ylim(0, synchronized_performance_data['messages_per_interval'].max() * 1.1)
        message_rate_axis.legend(loc='upper left', fontsize=self.style_configuration['legend_font_size'], frameon=True, fancybox=True, shadow=True, facecolor='white', edgecolor='gray', framealpha=0.9)
        message_rate_axis.set_title('Message Processing Rate Over Time', fontsize=self.style_configuration['title_font_size'], fontweight='bold', color=self.visualization_colors['text'], pad=20)
        message_rate_axis.tick_params(axis='both', labelsize=self.style_configuration['tick_label_size'])
        message_rate_axis.grid(True, alpha=self.style_configuration['grid_alpha'], linestyle='--')
        message_rate_axis.set_axisbelow(True)
        message_rate_axis.set_facecolor(self.visualization_colors['background'])

        # Configure time axis for message rate plot
        message_rate_axis.set_xticks(hour_tick_positions)
        message_rate_axis.set_xticklabels([f'{int(h)}' for h in hour_tick_positions])

        # Add comprehensive message processing statistics
        minimum_message_rate = synchronized_performance_data['messages_per_interval'].min()
        mean_message_rate = synchronized_performance_data['messages_per_interval'].mean()
        maximum_message_rate = synchronized_performance_data['messages_per_interval'].max()
        
        message_processing_statistics = (
            " Message Processing Rate Metrics\n"
            "─────────────────────────────────\n"
            f"Minimum Rate : {minimum_message_rate:8.0f} msg/15min\n"
            f"Mean Rate    : {mean_message_rate:8.0f} msg/15min\n"
            f"Maximum Rate : {maximum_message_rate:8.0f} msg/15min\n"
        )
        
        self._add_stats_text(message_rate_axis, message_processing_statistics, position=(0.77, 0.93))

        # Format axes for improved readability
        cpu_usage_axis.yaxis.set_major_formatter(plt.FuncFormatter(lambda x, p: f'{x:.1f}'))
        message_rate_axis.yaxis.set_major_formatter(plt.FuncFormatter(lambda x, p: f'{x:.0f}'))

        self._save_plot('cpu_load_vs_message_throughput.png')

        # Calculate and report correlation metrics for analysis insights
        cpu_usage_series = synchronized_performance_data['cpu_percent']
        message_rate_series = synchronized_performance_data['messages_per_interval']
        pearson_correlation_coefficient = cpu_usage_series.corr(message_rate_series)
        
        print(f"   Pearson Correlation (CPU Usage vs Message Rate): {pearson_correlation_coefficient:.3f}")

    def visualize_cryptocurrency_correlation_matrix(self):
        """
        Create comprehensive correlation heatmap showing cross-cryptocurrency relationships.
        
        Generates a professional heatmap visualization displaying average correlation
        coefficients between different cryptocurrency trading pairs, providing insights
        into market interdependencies and price movement relationships.
        """
        # Load correlation data for all supported trading symbols
        cryptocurrency_correlation_data = {}
        for trading_symbol in self.supported_trading_symbols:
            correlation_file_path = self.data_directory / 'metrics' / 'correlations' / f'{trading_symbol}.csv'
            if correlation_file_path.exists():
                correlation_dataframe = pd.read_csv(correlation_file_path)
                correlation_dataframe['trading_symbol'] = trading_symbol
                cryptocurrency_correlation_data[trading_symbol] = correlation_dataframe

        if not cryptocurrency_correlation_data:
            print("No cryptocurrency correlation data available for visualization")
            return

        # Set up consistent plotting style
        self._setup_plot_style()

        # Create figure for correlation heatmap
        figure, correlation_axis = plt.subplots(1, 1, figsize=self.style_configuration['medium_figure_size'])

        # Build correlation matrix from individual correlation measurements
        correlation_matrix = np.zeros((len(self.supported_trading_symbols), len(self.supported_trading_symbols)))
        correlation_count_matrix = np.zeros((len(self.supported_trading_symbols), len(self.supported_trading_symbols)))

        for base_symbol_index, base_trading_symbol in enumerate(self.supported_trading_symbols):
            if base_trading_symbol in cryptocurrency_correlation_data:
                correlation_dataframe = cryptocurrency_correlation_data[base_trading_symbol]
                
                for _, correlation_record in correlation_dataframe.iterrows():
                    if 'correlated_with' in correlation_record and correlation_record['correlated_with'] in self.supported_trading_symbols:
                        correlated_symbol_index = self.supported_trading_symbols.index(correlation_record['correlated_with'])
                        
                        if not np.isnan(correlation_record['correlation']):
                            correlation_matrix[base_symbol_index, correlated_symbol_index] += correlation_record['correlation']
                            correlation_count_matrix[base_symbol_index, correlated_symbol_index] += 1

        # Calculate average correlations where data exists
        valid_correlation_mask = correlation_count_matrix > 0
        correlation_matrix[valid_correlation_mask] = (
            correlation_matrix[valid_correlation_mask] / correlation_count_matrix[valid_correlation_mask]
        )
        correlation_matrix[~valid_correlation_mask] = np.nan

        # Create professional correlation heatmap
        correlation_heatmap = sns.heatmap(
            correlation_matrix,
            annot=True, fmt='.2f', cmap='RdBu_r', vmin=-1, vmax=1,
            xticklabels=[symbol.replace('-USDT', '') for symbol in self.supported_trading_symbols],
            yticklabels=[symbol.replace('-USDT', '') for symbol in self.supported_trading_symbols],
            ax=correlation_axis,
            cbar_kws={'shrink': 0.8, 'aspect': 30, 'label': 'Correlation Coefficient'},
            linewidths=0.5, linecolor='white', square=True,
            annot_kws={
                'size': self.style_configuration['tick_label_size'] - 2, 
                'weight': 'bold', 
                'color': self.visualization_colors['text']
            }
        )

        # Configure title and axis labels with professional formatting
        correlation_axis.set_title(
            'Cryptocurrency Cross-Correlation Analysis',
            fontsize=self.style_configuration['title_font_size'], fontweight='bold',
            color=self.visualization_colors['text'], pad=30
        )
        correlation_axis.set_xlabel(
            'Correlated Cryptocurrency Symbol', 
            fontsize=self.style_configuration['axis_label_size'],
            color=self.visualization_colors['text'], labelpad=15
        )
        correlation_axis.set_ylabel(
            'Base Cryptocurrency Symbol', 
            fontsize=self.style_configuration['axis_label_size'],
            color=self.visualization_colors['text'], labelpad=15
        )

        # Configure tick labels for optimal readability
        correlation_axis.tick_params(
            axis='x', rotation=45, labelsize=self.style_configuration['tick_label_size']
        )
        correlation_axis.tick_params(
            axis='y', rotation=0, labelsize=self.style_configuration['tick_label_size']
        )

        # Apply subtle background styling
        correlation_axis.set_facecolor(self.visualization_colors['background'])
        correlation_axis.grid(False)  # Disable grid for cleaner heatmap appearance
        correlation_axis.set_axisbelow(True)

        # Configure colorbar with consistent styling
        correlation_colorbar = correlation_heatmap.collections[0].colorbar
        correlation_colorbar.set_label(
            'Correlation Coefficient', 
            fontsize=self.style_configuration['axis_label_size'] - 2,
            color=self.visualization_colors['text']
        )
        correlation_colorbar.ax.tick_params(labelsize=self.style_configuration['tick_label_size'])

        # Save high-quality correlation visualization
        self._save_plot('cryptocurrency_price_correlation_matrix.png')
    
    def generate_comprehensive_analysis_suite(self):
        """
        Generate complete suite of trading system performance visualizations.
        
        Creates and saves the following analysis charts:
        1. Scheduler drift time series and distribution analysis
        2. Network and processing latency breakdown by time
        3. System resource utilization (CPU idle and memory usage)
        4. CPU usage correlation with message processing throughput
        5. Cross-cryptocurrency price correlation heatmap
        
        All generated visualizations are saved to the configured output directory.
        """
        print("=" * 70)
        print("CRYPTOCURRENCY TRADING SYSTEM PERFORMANCE ANALYSIS")
        print("=" * 70)
        print(f"Data Source: {self.data_directory.absolute()}")
        print(f"Output Directory: {self.output_directory.absolute()}")
        print()
        
        analysis_steps = [
            ("Scheduler Timing Drift Analysis", self.visualize_scheduler_drift_patterns),
            ("Network and Processing Latency Breakdown", self.visualize_latency_component_analysis),
            ("System Resource Utilization Analysis", self.visualize_cpu_memory_performance),
            ("CPU Load vs Message Throughput Analysis", self.visualize_cpu_load_message_throughput_correlation),
            ("Cross-Cryptocurrency Correlation Analysis", self.visualize_cryptocurrency_correlation_matrix)
        ]
        
        for step_number, (analysis_name, analysis_function) in enumerate(analysis_steps, 1):
            print(f"{step_number}. {analysis_name}...")
            try:
                analysis_function()
                print(f"   ✓ Completed successfully")
            except Exception as error:
                print(f"   ✗ Failed: {error}")
            print()

        print("=" * 70)
        print("ANALYSIS COMPLETE!")
        print(f"All visualizations saved to: {self.output_directory}")
        print("=" * 70)

def main():
    """
    Main execution function for the trading system visualization script.
    
    Initializes the visualizer with the specified data directory and generates
    all available performance analysis visualizations.
    """
    # Initialize the cryptocurrency trading system visualizer
    performance_visualizer = CryptocurrencyTradingSystemVisualizer('data/62-hours/data') 
    
    # Generate comprehensive performance analysis visualizations
    performance_visualizer.generate_comprehensive_analysis_suite()

if __name__ == "__main__":
    main()