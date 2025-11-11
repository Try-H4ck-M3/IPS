#!/bin/bash

# Enhanced build script
set -e  # Exit on any error

# Configuration
BUILD_DIR="./build"
TARGET="$BUILD_DIR/IPS"
SOURCES=(
    "./src/IPS.cpp"
    "./src/packets/packet_processor.cpp"
    "./src/utils/utils.cpp"
    "./src/logger/logger.cpp"
    "./src/rules/parser.cpp"
    "./src/config/config_parser.cpp"
    "./src/rules/expression_parser.cpp"
    "./src/rate_limit/rate_limiter.cpp"
)
LIBS=("-lnetfilter_queue" "-lpthread" "-lyaml-cpp")

# Build flags
#CXXFLAGS="-Wall -Wextra -O2 -s"
CXXFLAGS="-O2 -s"

# Create build directory
mkdir -p "$BUILD_DIR"

# Build command
echo "Started build"
g++ $CXXFLAGS -o "$TARGET" "${SOURCES[@]}" "${LIBS[@]}"

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "Build successful! Output: $TARGET"
    # Optional: display file size
    echo "File size: $(du -h "$TARGET" | cut -f1)"
else
    echo "Build failed!"
    exit 1
fi