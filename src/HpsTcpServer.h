
// Copyright (C) Stanislav Cherkasov

#ifndef HPSTCPSERVER_H_e580d236_740d_4857_b3f7_be43482abf76
#define HPSTCPSERVER_H_e580d236_740d_4857_b3f7_be43482abf76

#include "HpsConfig.h"
#include "HpsProgArgs.h"

namespace Hps
{
    class TCPServer
    {
    public:
        // c-tor
        TCPServer();

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

#endif // HPSTCPSERVER_H_e580d236_740d_4857_b3f7_be43482abf76
