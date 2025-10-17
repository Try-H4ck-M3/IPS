#pragma once
#include "../includes.h"
#include "../packets/packet_processor.h"
#include "../logger/logger.h"

extern struct nfq_handle *h;
extern struct nfq_q_handle *qh;
extern int fd;

int prepare(Logger logger);
void setup_iptables();
void cleanup_iptables();
void cleanup_and_exit(int sig);