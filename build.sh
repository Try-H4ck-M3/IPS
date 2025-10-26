#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build

g++ -s -o ./build/IPS \
    ./IPS.cpp \
    ./packets/packet_processor.cpp \
    ./utils/utils.cpp \
    ./logger/logger.cpp \
    ./rules/parser.cpp \
    ./rules/expression_parser.cpp \
    -lnetfilter_queue 