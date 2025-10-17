#pragma once
#include "../includes.h"

class Logger {
public:
    Logger(bool verbose);
    int is_verbose();
    int info(string message);
    int warn(string message);
    int error(string message);
    int verbose_log(string message);
    int alert(string message);
private:
    string get_current_time();
    bool verbose;
};