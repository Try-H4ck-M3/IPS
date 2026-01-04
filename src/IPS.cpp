#include "includes.h"
#include "utils/utils.h"
#include "packets/packet_processor.h"
#include "logger/logger.h"
#include "rate_limit/rate_limiter.h"

#include "rules/parser.h"
#include "config/config_parser.h"

#include <tclap/CmdLine.h>

// Global rate limiter
RateLimiter* g_rate_limiter = nullptr;

int main(int argc, char** argv)
{
    TCLAP::CmdLine cmd("IPS", ' ', "0.1");
    TCLAP::SwitchArg debug_arg("v", "verbose", "Verbose mode", false);
    TCLAP::ValueArg<string> configpath_arg("c", "config", "./config/config.yml", true, "", "string");
    cmd.add(debug_arg);
    cmd.add(configpath_arg);

    try {
        cmd.parse(argc, argv);

        bool verbose = debug_arg.getValue();
        string configpath = configpath_arg.getValue();

        // Create a temporary logger for initial config loading
        Logger temp_logger(verbose);
        set_packet_processor_logger(&temp_logger);
        
        // Initialize the global rate limiter
        g_rate_limiter = new RateLimiter();
        
        prepare(temp_logger);
        temp_logger.info("Loading config");
        ConfigParser parser(configpath);
        if (!parser.loadConfig(&temp_logger))
        {
            cerr << "Failed to load configuration!" << endl;
            return 1;
        }
        if (verbose)
        {
            parser.printAll();
        }
        
        // Set rules path from config
        g_rules_path = parser.get("rules.path", "./configs/rules.json");
        
        // Get logpath from config and create logger with file logging
        string logpath = parser.get("rules.logpath", "");
        Logger logger(verbose, logpath);
        set_packet_processor_logger(&logger);
        
        // Load rules early to show them in verbose mode
        load_rules_at_startup();

        cout << endl;
        logger.info("IPS started successfully");

        fd = nfq_fd(h);
        char buf[4096] __attribute__((aligned));
        int rv;
        while ((rv = recv(fd, buf, sizeof(buf), 0)) >= 0)
        {
            nfq_handle_packet(h, buf, rv);
        }

        cleanup_and_exit(0);
        return 0;
    }

    catch (TCLAP::ArgException &e)
    {
        cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
        return 1;
    }
}

