# Makefile for OKX Real-time Trade Processor
# Author: Fraidakis Ioannis

# =============================================================================
# CONFIGURATION
# =============================================================================

# Compiler settings
CC = gcc
ARM_CC = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread -O2 -g
LDFLAGS = -pthread -lwebsockets -lm

# Directories
SRC_DIR = src
INCLUDE_DIR = include
DATA_DIR = data

# Find all source files automatically
SRCS = $(shell find $(SRC_DIR) -name "*.c")
OBJS = $(SRCS:$(SRC_DIR)/%.c=build/%.o)
ARM_OBJS = $(SRCS:$(SRC_DIR)/%.c=build-arm/%.o)

# Targets
TARGET = main
ARM_TARGET = main-arm

# Include paths
INCLUDES = -I$(INCLUDE_DIR) -I$(SRC_DIR)

# Deployment settings
DEPLOY_HOST = fraidaki@pi-zero.local
DEPLOY_DIR = /home/okx-trader
FETCH_DIR = fetched-data

# =============================================================================
# BUILD TARGETS
# =============================================================================

.PHONY: all clean clean-arm clean-all arm run background kill deploy deploy-arm fetch help

# Default target
all: $(TARGET)

# Native build
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Built successfully: $(TARGET)"

build/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# ARM cross-compilation
arm: $(ARM_TARGET)

$(ARM_TARGET): $(ARM_OBJS)
	$(ARM_CC) $(ARM_OBJS) -o $(ARM_TARGET) $(LDFLAGS)
	@echo "Cross-compiled successfully: $(ARM_TARGET)"

build-arm/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(ARM_CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# =============================================================================
# UTILITIES
# =============================================================================

# Clean targets
clean:
	rm -rf build $(TARGET)
	@echo "Cleaned build artifacts"

clean-arm:
	rm -rf build-arm $(ARM_TARGET)
	@echo "Cleaned ARM build artifacts"

clean-all: clean clean-arm
	rm -rf $(DATA_DIR) output.log
	@echo "Cleaned all including data files"

# Run targets
run: $(TARGET)
	./$(TARGET)

# Background execution
background: $(TARGET)
	nohup ./$(TARGET) > output.log 2>&1 &
	@echo "Started in background. Check output.log for logs."

# Kill process
kill:
	@if pgrep -x "$(TARGET)" > /dev/null 2>&1; then \
		pkill -TERM -x "$(TARGET)" && echo "Killed running instances of $(TARGET)."; \
	else \
		echo "No running instances of $(TARGET) found."; \
	fi
	
# =============================================================================
# DEPLOYMENT
# =============================================================================

# Deploy ARM binary
deploy: arm
	rsync -avz --progress $(ARM_TARGET) $(DEPLOY_HOST):$(DEPLOY_DIR)/$(TARGET)
	rsync -avz --progress Makefile $(DEPLOY_HOST):$(DEPLOY_DIR)/
	@echo "Deployed ARM binary and Makefile to $(DEPLOY_HOST):$(DEPLOY_DIR)/"

# Fetch output files from Raspberry Pi
fetch:
	mkdir -p $(FETCH_DIR)
	rsync -avz --progress $(DEPLOY_HOST):$(DEPLOY_DIR)/$(DATA_DIR)/ $(FETCH_DIR)/$(DATA_DIR)/
	rsync -avz --progress $(DEPLOY_HOST):$(DEPLOY_DIR)/nohup.out $(FETCH_DIR)/nohup.out

# =============================================================================
# HELP
# =============================================================================

help:
	@echo "Available targets:"
	@echo "  all		 - Build the program (default)"
	@echo "  arm		 - Cross-compile for ARM architecture"
	@echo "  clean		 - Remove build artifacts"
	@echo "  clean-arm	 - Remove ARM build artifacts"
	@echo "  clean-all	 - Remove all build artifacts and data files"
	@echo "  run		 - Build and run the program"
	@echo "  background	 - Build and run in background"
	@echo "  kill		 - Kill running instances"
	@echo "  deploy    	 - Deploy ARM binary and Makefile to Raspberry Pi"
	@echo "  fetch		 - Fetch data from Raspberry Pi"
	@echo "  help		 - Show this help message"
