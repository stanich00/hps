
// Copyright (C) Stanislav Cherkasov

#ifndef HPSTCPSERVER_H_E580D236_740D_4857_B3F7_BE43482ABF76
#define HPSTCPSERVER_H_E580D236_740D_4857_B3F7_BE43482ABF76

#include "HpsConfig.h"
#include "HpsProgArgs.h"

namespace Hps
{
    class TCPServer
    {
    public:
        // run server
        int Run(ProgArgs const& progArgs);

    protected:
        // start listening incoming connections
        int Listen()const;

        // process every new connection
        void Accept(int fdSocket)const;

        // handle connection
        int HandleConnection(int fd, ConfigPtr const& config)const;

    private:
        ConfigPtr m_config;
    };
} // namespace Hps

#endif // HPSTCPSERVER_H_E580D236_740D_4857_B3F7_BE43482ABF76
