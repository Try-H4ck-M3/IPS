#include "logger.h"

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"

Logger::Logger(bool verbose)
{
    this->verbose = verbose;
}

int Logger::is_verbose()
{
    return verbose;
}

int Logger::info(string message)
{
    cout << get_current_time() + " - " + BLUE + "INFO" + RESET + " - " << message << endl;
    return 0;
}

int Logger::warn(string message)
{
    cout << get_current_time() + " - " + YELLOW + "WARN" + RESET + " - " << message << endl;
    return 0;
}

int Logger::error(string message)
{
    cout << get_current_time() + " - " + RED + "ERR" + RESET + "  - " << message << endl;
    return 0;
}

int Logger::verbose_log(string message)
{
    if (verbose)
    {
        cout << get_current_time() + " - " + MAGENTA + "VERB" + RESET + " - " << message << endl;
    }
    return 0;
}

int Logger::alert(string message)
{
    cout << get_current_time() + " - " + RED + "ALRT" + RESET + " - " << message << endl;
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