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
            cout << "| Rule ID    : " << rule_id << "\n";
            cout << "|   Desc     : " << description << "\n";
            cout << "|   Src IP   : " << src_ip << "\n";
            cout << "|   Dst IP   : " << dst_ip << "\n";
            
            cout << "|   Src Port : " << src_port << "\n";
            cout << "|   Dst Port : " << dst_port << "\n";
            
            cout << "|   Protocol : " << protocol << "\n";
            cout << "|   Action   : " << action << "\n";
            if (!string_content.empty())
            {
                cout << "|   String   : " << string_content << "\n";
            }
            if (is_rate_limit_rule)
            {
                cout << "|" << endl;
                cout << "|   Rate Limit   : " << max_requests << " requests per " << time_window_seconds << " seconds\n";
                cout << "|   Ban Duration : " << ban_duration_seconds << " seconds\n";
            }
            cout << "\n";
        }
    }
};
vector<Rule> parse_all_rules(string rules_file_path, Logger logger);