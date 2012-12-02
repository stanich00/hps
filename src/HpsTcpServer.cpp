
// Copyright (C) Stanislav Cherkasov

#include "HpsTcpServer.h"
#include "HpsCommon.h"
#include <memory.h>
#include <strings.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <cassert>
#include <set>
#include <vector>
#include <string>

namespace Hps
{
    int TCPServer::Run(ProgArgs const& progArgs)
    {
        try
        {
            // read config file and create list of parameters
            ProgArgs::List const& argList = progArgs.GetArgs();
            m_config.reset(new Config(argList.size() > 1? argList[1]: ""));

            // start listening for client connections
            int const fd = Listen();

            // process every new cient connection
            Accept(fd);
        }
        catch(std::exception const& ex)
        {
            GetLog().Msg(Log::Fatal, "%s", ex.what());
            return EXIT_FAILURE;
        }

        return 0;
    }

    int TCPServer::Listen()const
    {
        int fd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
        if(fd == -1)
            throw std::runtime_error("Error in socket opening");

        int yes = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        int const port = m_config->GetIntParam("SERVER_PORT");
        if(port == 0)
            throw std::runtime_error("Port number is incorrect: 0");

        struct sockaddr_in stSockAddr;
        memset(&stSockAddr, 0, sizeof(stSockAddr));
        stSockAddr.sin_family = PF_INET;
        stSockAddr.sin_port = htons(port);
        stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if(bind(fd, (sockaddr*)&stSockAddr, sizeof(stSockAddr)) == -1)
        {
            close(fd);
            throw std::runtime_error("Error in binding");
        }

        if(listen(fd, 10) == -1)
        {
            close(fd);
            throw std::runtime_error("Error in listening");
        }

        GetLog().Msg(Log::Info, "Listening on port: %d", port);

        return fd;
    }

    void TCPServer::Accept(int fdSocket)const
    {
        // client connection descriptors
        std::set<int> clients;

        for(;;)
        {
            // initialize read connection set
            fd_set readFdSet;
            FD_ZERO(&readFdSet);
            FD_SET(fdSocket, &readFdSet);

            for(std::set<int>::iterator it = clients.begin(), ti = clients.end(); it != ti; ++it)
                FD_SET(*it, &readFdSet);

            // init timeout
            timeval timeout;
            memset(&timeout, 0, sizeof(timeout));
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;

            // wait for an event in one of connections
            int maxFd = std::max(fdSocket, *std::max_element(clients.begin(), clients.end()));
            int readyFd = select(maxFd + 1, &readFdSet, 0, 0, &timeout);
            if(readyFd < 0)
            {
                shutdown(fdSocket, SHUT_RDWR);
                close(fdSocket);
                throw std::runtime_error("Error in select");
            }

            // if there are no ready for reading descriptors, continue
            if(readyFd == 0)
                continue;

            // check if there are new connection
            if(FD_ISSET(fdSocket, &readFdSet))
            {
                struct sockaddr_storage addr;
                socklen_t addrlen = 0;
                memset(&addr, 0, sizeof(addr));

                // accept new connection
                int fdConn = accept(fdSocket, (struct sockaddr*)&addr, &addrlen);
                if(fdConn < 0)
                {
                    shutdown(fdConn, SHUT_RDWR);
                    close(fdConn);
                    throw std::runtime_error("Error in accepting");
                }

                // set non-blocking mode to new connection
                int const flags = fcntl(fdConn, F_GETFL, 0);
                if(flags < 0)
                    throw std::runtime_error("F_GETFL error (client)");
                if(fcntl(fdConn, F_SETFL, flags | O_NONBLOCK) < 0)
                    throw std::runtime_error("F_SETFL error (client)");

                // add new connection descriptor
                clients.insert(fdConn);
            }

            // handle connections
            std::vector<int> toRemove;
            toRemove.reserve(clients.size());
            for(std::set<int>::const_iterator it = clients.begin(), ti = clients.end(); it != ti; ++it)
            {
                if(FD_ISSET(*it, &readFdSet))
                {
                    // handle connection (read/write data)
                    if(HandleConnection(*it, m_config) != 0)
                        GetLog().Msg(Log::Error, "Could not handle connection");

                    // close connection
                    shutdown(*it, SHUT_RDWR);
                    close(*it);

                    // put connection descriptor to remove it later on
                    toRemove.push_back(*it);
                }
            }
            // remove handled connection descriptors
            for(std::vector<int>::const_iterator it = toRemove.begin(), ti = toRemove.end(); it != ti; ++it)
                clients.erase(*it);
        }
    }

    int TCPServer::HandleConnection(int fd, ConfigPtr const& /*config*/)const
    {
        char buffer[bufferSize + 1];
        //char buf[bufferSize + 1];
        memset(buffer, 0, sizeof(buffer));
        size_t readSize = 0;

        while(readSize < bufferSize)
        {
            // read data from client
            int n = read(fd, buffer + readSize, sizeof(buffer) - 1);

            // check result of reading
            if(n <= 0)
                break; // all data were read

            readSize += n;

            usleep(100);
        }
        buffer[readSize] = 0;

        try
        {
            // check request
            if(strcasecmp(buffer, "hello") != 0)
            {
                GetLog().Msg(Log::Error, "Protocol error\nUnknown command: %s");
                return 1;
            }

            // send response
            std::string data = "OK";

            size_t writeSize = 0;
            while(writeSize < data.length())
            {
                int n = write(fd, data.c_str(), data.length());
                if(n <= 0)
                    break; // all data were written

                writeSize += n;

                usleep(100);
            }
        }
        catch(std::exception const& e)
        {
            GetLog().Msg(Log::Error, "%s", e.what());
            return 1;
        }

        return 0;
    }
}
