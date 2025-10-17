#pragma once
#include "../includes.h"
#include "../logger/logger.h"
#include "../utils/utils.h"

struct Rule {
    int rule_id;
    string description;
    string src_ip;
    string dst_ip;
    int src_port;
    int dst_port;
    string protocol;
    string action;

    // Optional: method to print rule (only in verbose mode)
    void print(Logger& logger) const
    {
        if (logger.is_verbose())
        {
            cout << "| Rule ID    : " << rule_id << "\n";
            cout << "|   Desc     : " << description << "\n";
            cout << "|   Src IP   : " << src_ip << "\n";
            cout << "|   Dst IP   : " << dst_ip << "\n";
            
            if (src_port == -1)
            {
                cout << "|   Src Port : " << "any" << "\n";
            }
            else
            {
                cout << "|   Src Port : " << src_port << "\n";
            }
            
            if (dst_port == -1)
            {
                cout << "|   Dst Port : " << "any" << "\n";
            }
            else
            {
                cout << "|   Dst Port : " << dst_port << "\n";
            }
            
            cout << "|   Protocol : " << protocol << "\n";
            cout << "|   Action   : " << action << "\n\n";
        }
    }
};
vector<Rule> parse_all_rules(string rules_file_path, Logger logger);