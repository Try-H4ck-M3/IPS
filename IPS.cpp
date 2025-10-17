#include "includes.h"
#include "utils/utils.h"
#include "packets/packet_processor.h"
#include "logger/logger.h"
#include "rules/parser.h"

#include <tclap/CmdLine.h>

int main(int argc, char** argv)
{
    TCLAP::CmdLine cmd("IPS", ' ', "0.1");
    TCLAP::SwitchArg debug_arg("v", "verbose", "Verbose mode", false);
    cmd.add(debug_arg);
    cmd.parse(argc, argv);

    bool verbose = debug_arg.getValue();

    Logger logger(verbose);
    set_packet_processor_logger(&logger);
    prepare(logger);

    cout << endl;
    logger.info("IPS started successfully.");

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

