#include "logger.h"
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"

Logger::Logger(bool verbose, const string& logpath)
{
    this->verbose = verbose;
    this->logpath = logpath;
    
    if (!logpath.empty())
    {
        ensure_directory_exists(logpath);
        logfile.open(logpath, ios::app);
        if (!logfile.is_open())
        {
            cerr << "Warning: Could not open log file: " << logpath << endl;
        }
    }
}

Logger::~Logger()
{
    if (logfile.is_open())
    {
        logfile.close();
    }
}

void Logger::ensure_directory_exists(const string& filepath)
{
    size_t last_slash = filepath.find_last_of("/\\");
    if (last_slash != string::npos)
    {
        string dir = filepath.substr(0, last_slash);
        struct stat info;
        if (stat(dir.c_str(), &info) != 0)
        {
            // Directory doesn't exist, create it
            #ifdef _WIN32
                _mkdir(dir.c_str());
            #else
                mkdir(dir.c_str(), 0755);
            #endif
        }
    }
}

void Logger::write_log(const string& level, const string& message, bool use_color)
{
    string timestamp = get_current_time();
    string log_line = timestamp + " - " + level + " - " + message;
    
    // Write to console with colors
    if (use_color)
    {
        string color = "";
        // Match level without trailing spaces
        if (level.find("INFO") == 0) color = BLUE;
        else if (level.find("WARN") == 0) color = YELLOW;
        else if (level.find("ERR") == 0 || level.find("ALRT") == 0) color = RED;
        else if (level.find("VERB") == 0) color = MAGENTA;
        
        cout << timestamp + " - " + color + level + RESET + " - " << message << endl;
    }
    else
    {
        cout << log_line << endl;
    }
    
    // Write to file (without colors)
    if (logfile.is_open())
    {
        logfile << log_line << endl;
        logfile.flush();
    }
}

int Logger::is_verbose()
{
    return verbose;
}

int Logger::info(string message)
{
    write_log("INFO", message);
    return 0;
}

int Logger::warn(string message)
{
    write_log("WARN", message);
    return 0;
}

int Logger::error(string message)
{
    write_log("ERR ", message);
    return 0;
}

int Logger::verbose_log(string message)
{
    if (verbose)
    {
        write_log("VERB", message);
    }
    return 0;
}

int Logger::alert(string message)
{
    write_log("ALRT", message);
    return 0;
}

int Logger::write_raw(string message)
{
    // Write to console
    cout << message << endl;
    
    // Write to file
    if (logfile.is_open())
    {
        logfile << message << endl;
        logfile.flush();
    }
    return 0;
}

string Logger::get_current_time()
{
    auto now = chrono::system_clock::now();
    
    time_t time_t_now = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    
    return ss.str();
}