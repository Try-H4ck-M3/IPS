#pragma once
#include "../includes.h"
#include "../logger/logger.h"
#include "../utils/utils.h"
#include "expression_parser.h"

struct Rule {
    int rule_id;
    string description;
    string src_ip;
    string dst_ip;
    string src_port;
    string dst_port;
    string protocol;
    string action;
    string string_content;
    string regex_content;
    
    // Rate limiting fields
    bool is_rate_limit_rule;
    int max_requests;
    int time_window_seconds;
    int ban_duration_seconds;

    // Optional: method to print rule (only in verbose mode)
    void print(Logger& logger) const
    {
        if (logger.is_verbose())
        {
            logger.write_raw("| Rule ID    : " + to_string(rule_id));
            logger.write_raw("|   Desc     : " + description);
            logger.write_raw("|   Src IP   : " + src_ip);
            logger.write_raw("|   Dst IP   : " + dst_ip);
            logger.write_raw("|   Src Port : " + src_port);
            logger.write_raw("|   Dst Port : " + dst_port);
            logger.write_raw("|   Protocol : " + protocol);
            logger.write_raw("|   Action   : " + action);
            if (!string_content.empty())
            {
                logger.write_raw("|   String   : " + string_content);
            }
            if (!regex_content.empty())
            {
                logger.write_raw("|   Regex    : " + regex_content);
            }
            if (is_rate_limit_rule)
            {
                logger.write_raw("|");
                logger.write_raw("|   Rate Limit   : " + to_string(max_requests) + " requests per " + to_string(time_window_seconds) + " seconds");
                logger.write_raw("|   Ban Duration : " + to_string(ban_duration_seconds) + " seconds");
            }
            logger.write_raw("");
        }
    }
};
vector<Rule> parse_all_rules(string rules_file_path, Logger& logger);