#pragma once
#include "../includes.h"
#include "../rules/parser.h"
#include "../logger/logger.h"

class Logger; // forward declare

void set_packet_processor_logger(Logger *logger);

bool packet_processor(unsigned char *data, int len, string src_ip, string dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t protocol); // 6 = TCP, 17 = UDP