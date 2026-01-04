#pragma once
#include "../includes.h"

class Logger {
public:
    Logger(bool verbose, const string& logpath = "");
    ~Logger();
    int is_verbose();
    int info(string message);
    int warn(string message);
    int error(string message);
    int verbose_log(string message);
    int alert(string message);
    int write_raw(string message);  // Write raw line to both console and file (for detailed output)
private:
    string get_current_time();
    void write_log(const string& level, const string& message, bool use_color = true);
    void ensure_directory_exists(const string& filepath);
    bool verbose;
    string logpath;
    ofstream logfile;
};