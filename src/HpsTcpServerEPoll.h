
// Copyright (C) Stanislav Cherkasov

#ifndef HPSTCPSERVEREPOLL_H_92E51662_4E73_4B73_8288_84D5094FAB7E
#define HPSTCPSERVEREPOLL_H_92E51662_4E73_4B73_8288_84D5094FAB7E

#include "HpsConfig.h"
#include "HpsProgArgs.h"

namespace Hps
{
    class TcpServerEPoll
    {
    public:
        // run server
        int Run(ProgArgs const& progArgs);

    protected:
        // start listening incoming connections
        int Listen()const;

        // process every new connection
        void Accept(int sfd)const;

        // handle connection
        int HandleConnection(int fd, ConfigPtr const& config)const;
        int HandleConnection2(int fd, ConfigPtr const& config)const;

        // make socket non-blocking
        void MakeNoneBlocking(int fd)const;

    private:
        ConfigPtr m_config;
    };
} // namespace Hps

#endif // HPSTCPSERVEREPOLL_H_92E51662_4E73_4B73_8288_84D5094FAB7E
