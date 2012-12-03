
// Copyright (C) Stanislav Cherkasov

#include "HpsTcpServerEPoll.h"
#include "HpsCommon.h"
#include <memory.h>
#include <strings.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <cassert>
#include <set>
#include <vector>
#include <string>


namespace Hps
{
    int TcpServerEPoll::Run(ProgArgs const& progArgs)
    {
        try
        {
            // get config file name from program arguments
            // use default config file name if there are no program arguments
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

    int TcpServerEPoll::Listen()const
    {
        // this class is to automatically call freeaddrinfo
        struct AddrInfoHolder
        {
            AddrInfoHolder(): m_addrinfo(0){}
            ~AddrInfoHolder(){ freeaddrinfo(m_addrinfo); m_addrinfo = 0; }
             addrinfo *m_addrinfo;
        };

        // get settings (port)
        std::string const sport = m_config->GetStrParam("SERVER_PORT");
        // check if port is correct
        if(sport.empty())
            throw std::runtime_error("Port number is incorrect: 0");

        // get addr info
        addrinfo hints;
        memset (&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        AddrInfoHolder result; // addrinfo*
        int const r = getaddrinfo(0, sport.c_str(), &hints, &result.m_addrinfo);
        if (r != 0)
            throw std::runtime_error(std::string("Error in socket getaddrinfo: ") + gai_strerror(r));

        // create socket and bind it
        int fd = 0;
        for (addrinfo * rp = result.m_addrinfo; rp != 0; rp = rp->ai_next)
        {
            fd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (fd == -1)
                continue;

            int yes = 1;
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

            if(bind (fd, rp->ai_addr, rp->ai_addrlen) == 0)
                break; // success

            close(fd);
            fd = 0;
        }

        if(fd == 0)
            throw std::runtime_error("Error in creating socket or binding");

        // set socket non-blocking
        MakeNoneBlocking(fd);

        // listen socket
        if(listen(fd, 10) == -1)
        {
            close(fd);
            throw std::runtime_error("Error in listening");
        }

        GetLog().Msg(Log::Info, "Listening on port: %s", sport.c_str());
        return fd;
    }

    void TcpServerEPoll::Accept(int sfd)const
    {
        int efd = epoll_create1(0);
        if (efd == -1)
            throw std::runtime_error("Error in epoll_create1");

        epoll_event event;
        memset(&event, 0, sizeof(event));
        event.data.fd = sfd;
        event.events = EPOLLIN | EPOLLET;
        if(epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event) == -1)
            throw std::runtime_error("Error in epoll_ctl");

        // TODO: here threads can be created
        int const nMaxEventCount = m_config->GetIntParam("MAX_EVENT_COUNT");
        assert(nMaxEventCount > 0);
        std::vector<epoll_event> events(nMaxEventCount); // buffer where events are returned
        for(;;)
        {
            int const n = epoll_wait (efd, &events[0], nMaxEventCount, -1);
            for(int i = 0; i < n; ++i)
            {
                if((events[i].events & EPOLLERR)
                    || (events[i].events & EPOLLHUP)
                    || (!(events[i].events & EPOLLIN)))
                {
                    // an error has occured on this fd, or the socket is not ready for reading
                    GetLog().Msg(Log::Error, "Error in epoll_wait, continue");
                    close (events[i].data.fd);
                    continue;
                }
                else if(sfd == events[i].data.fd)
                {
                    // a notification on the listening socket (one or more incoming connections)
                    for(;;)
                    {
                        sockaddr in_addr;
                        memset(&in_addr, 0, sizeof(in_addr));
                        socklen_t in_len = sizeof(in_addr);

                        int const infd = accept (sfd, &in_addr, &in_len);
                        if (infd == -1)
                        {
                            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                            {
                                // We have processed all incoming connections
                                break;
                            }
                            else
                            {
                                GetLog().Msg(Log::Error, "Error in accept");
                                break;
                            }
                        }

                        // get info for debug output
                        if(GetLog().GetLevel() <= Log::Debug)
                        {
                            char hbuf[NI_MAXHOST];
                            char sbuf[NI_MAXSERV];
                            int const r = getnameinfo (&in_addr, in_len,
                                                       hbuf, sizeof hbuf,
                                                       sbuf, sizeof sbuf,
                                                       NI_NUMERICHOST | NI_NUMERICSERV);
                            if (r == 0)
                            {
                                GetLog().Msg(Log::Debug,
                                             "Accepted connection on descriptor %d (host=%s , port=%s)",
                                             infd, hbuf, sbuf);
                            }
                        }

                        // make the incoming socket non-blocking
                        MakeNoneBlocking(infd);

                        // register descriptor on epoll instance
                        event.data.fd = infd;
                        event.events = EPOLLIN | EPOLLET;
                        if(epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event) == -1)
                            throw(std::runtime_error("Error in epoll_ctl"));

                    } // for(;;)
                    continue;
                }
                else
                {
                    if(HandleConnection(events[i].data.fd, m_config) != 0)
                        GetLog().Msg(Log::Error, "Could not handle connection");

                    // close connection
                    shutdown(events[i].data.fd, SHUT_RDWR);
                    close(events[i].data.fd);
                }
            } // for(int ...)
        } // for(;;)

        close(efd);
        close(sfd);
    }

    int TcpServerEPoll::HandleConnection(int fd, ConfigPtr const& /*config*/)const
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

    void TcpServerEPoll::MakeNoneBlocking(int fd)const
    {
        int const flags = fcntl(fd, F_GETFL, 0);
        if(flags < 0)
            throw std::runtime_error("F_GETFL error (client)");
        if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
            throw std::runtime_error("F_SETFL error (client)");
    }
} // namespace Hps
