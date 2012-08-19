
// Copyright (C) Stanislav Cherkasov

#include "HpsLog.h"
#include "HpsStdOutLogWriter.h"
#include "HpsTcpServer.h"
#include "HpsProgArgs.h"
#include <memory>
#include <cassert>

namespace
{
    std::unique_ptr<Hps::Log> g_log;
}

int main(int argc, char * argv[])
{
    using namespace Hps;

    // create log
    g_log.reset(new Log(Log::Error, LogWriterPtr(new StdOutLogWriter())));

    // create server
    TCPServer server;

    // run server
    return server.Run(ProgArgs(argc, argv));
}

namespace Hps
{
    Hps::Log & GetLog()
    {
        assert(g_log);
        return *g_log;
    }
} // namespace Hps
