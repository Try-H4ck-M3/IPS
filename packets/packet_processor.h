#pragma once
#include "../includes.h"
#include "../rules/parser.h"
#include "../logger/logger.h"

// Forward declarations
class Logger;
class RateLimiter;

void set_packet_processor_logger(Logger *logger);
void load_rules_at_startup(); // Load rules early with proper verbose logging

bool packet_processor(unsigned char *data, int len, string src_ip, string dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t protocol); // 6 = TCP, 17 = UDP

// External reference to global rate limiter
extern RateLimiter* g_rate_limiter;