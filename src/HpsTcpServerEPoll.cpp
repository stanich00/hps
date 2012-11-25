
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
        std::string const sport = m_config->GetParam("SERVER_PORT");
        // check if port is correct
        int const port = atoi(sport.c_str());
        if(port == 0)
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

            if(bind (fd, rp->ai_addr, rp->ai_addrlen) == 0)
                break; // success

            close(fd);
            fd = 0;
        }

        if(fd == 0)
            throw std::runtime_error("Error in creating socket or binding");

        // set socket non-blocking
        int const flags = fcntl(fd, F_GETFL, 0);
        if(flags < 0)
            throw std::runtime_error("F_GETFL error (server)");
        if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
            throw std::runtime_error("F_SETFL error (server)");

        // listen socket
        if(listen(fd, 10) == -1)
        {
            close(fd);
            throw std::runtime_error("Error in listening");
        }

        GetLog().Msg(Log::Info, "Listening on port: %s", port);

        return fd;
    }

    void TcpServerEPoll::Accept(int sfd)const
    {
//        // client connection descriptors
//        std::set<int> clients;

//        for(;;)
//        {
//            // initialize read connection set
//            fd_set readFdSet;
//            FD_ZERO(&readFdSet);
//            FD_SET(fdSocket, &readFdSet);

//            for(std::set<int>::iterator it = clients.begin(), ti = clients.end(); it != ti; ++it)
//                FD_SET(*it, &readFdSet);

//            // init timeout
//            timeval timeout;
//            memset(&timeout, 0, sizeof(timeout));
//            timeout.tv_sec = 10;
//            timeout.tv_usec = 0;

//            // wait for an event in one of connections
//            int maxFd = std::max(fdSocket, *std::max_element(clients.begin(), clients.end()));
//            int readyFd = select(maxFd + 1, &readFdSet, 0, 0, &timeout);
//            if(readyFd < 0)
//            {
//                shutdown(fdSocket, SHUT_RDWR);
//                close(fdSocket);
//                throw std::runtime_error("Error in select");
//            }

//            // if there are no ready for reading descriptors, continue
//            if(readyFd == 0)
//                continue;

//            // check if there are new connection
//            if(FD_ISSET(fdSocket, &readFdSet))
//            {
//                struct sockaddr_storage addr;
//                socklen_t addrlen = 0;
//                memset(&addr, 0, sizeof(addr));

//                // accept new connection
//                int fdConn = accept(fdSocket, (struct sockaddr*)&addr, &addrlen);
//                if(fdConn < 0)
//                {
//                    shutdown(fdConn, SHUT_RDWR);
//                    close(fdConn);
//                    throw std::runtime_error("Error in accepting");
//                }

//                // set non-blocking mode to new connection
//                int const flags = fcntl(fdConn, F_GETFL, 0);
//                if(flags < 0)
//                    throw std::runtime_error("F_GETFL error (client)");
//                if(fcntl(fdConn, F_SETFL, flags | O_NONBLOCK) < 0)
//                    throw std::runtime_error("F_SETFL error (client)");

//                // add new connection descriptor
//                clients.insert(fdConn);
//            }

//            // handle connections
//            std::vector<int> toRemove;
//            toRemove.reserve(clients.size());
//            for(std::set<int>::const_iterator it = clients.begin(), ti = clients.end(); it != ti; ++it)
//            {
//                if(FD_ISSET(*it, &readFdSet))
//                {
//                    // handle connection (read/write data)
//                    if(HandleConnection(*it, m_config) != 0)
//                        GetLog().Msg(Log::Error, "Could not handle connection");

//                    // close connection
//                    shutdown(*it, SHUT_RDWR);
//                    close(*it);

//                    // put connection descriptor to remove it later on
//                    toRemove.push_back(*it);
//                }
//            }
//            // remove handled connection descriptors
//            for(std::vector<int>::const_iterator it = toRemove.begin(), ti = toRemove.end(); it != ti; ++it)
//                clients.erase(*it);
//        }

        int efd = epoll_create1(0);
        if (efd == -1)
            throw std::runtime_error("Error in epoll_create1");

        epoll_event event;
        memset(&event, 0, sizeof(event));
        event.data.fd = sfd;
        event.events = EPOLLIN | EPOLLET;
        if(epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event) == -1)
            throw std::runtime_error("Error in epoll_ctl");

        assert(nMaxEventCount > 0);
        std::vector<epoll_event> events(nMaxEventCount); // Buffer where events are returned
        //events = calloc (MAXEVENTS, sizeof event);
        for(;;)
        {
            int const n = epoll_wait (efd, &events[0], nMaxEventCount, -1);
            for(int i = 0; i < n; ++i)
            {
                if((events[i].events & EPOLLERR)
                    || (events[i].events & EPOLLHUP)
                    || (!(events[i].events & EPOLLIN)))
                {
                    // An error has occured on this fd, or the socket is not ready for reading
                    GetLog().Msg(Log::Error, "Error in epoll_wait, continue");
                    close (events[i].data.fd);
                    continue;
                }
                else if(sfd == events[i].data.fd)
                {
                    // We have a notification on the listening socket, which means one or more incoming connections
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

                        // Make the incoming socket non-blocking and add it to the list of fds to monitor
                        int const flags = fcntl(infd, F_GETFL, 0);
                        if(flags < 0)
                            throw std::runtime_error("F_GETFL error (client)");
                        if(fcntl(infd, F_SETFL, flags | O_NONBLOCK) < 0)
                            throw std::runtime_error("F_SETFL error (client)");

                        event.data.fd = infd;
                        event.events = EPOLLIN | EPOLLET;
                        if(epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event) == -1)
                            throw(std::runtime_error("Error in epoll_ctl"));
                    }
                    continue;
                }
                else
                {
                    // We have data on the fd waiting to be read. Read and
                    // display it. We must read whatever data is available
                    // completely, as we are running in edge-triggered mode
                    // and won't get a notification again for the same
                    // data
                    HandleConnection(events[i].data.fd, m_config);
                    if(HandleConnection(events[i].data.fd, m_config) != 0)
                        GetLog().Msg(Log::Error, "Could not handle connection");

                    // close connection
                    shutdown(events[i].data.fd, SHUT_RDWR);
                    close(events[i].data.fd);

                    // Handle connection
//                    int done = 0;

//                    for(;;)
//                    {
//                        ssize_t count = 0;
//                        char buf[512];

//                        count = read (events[i].data.fd, buf, sizeof(buf));
//                        if (count == -1)
//                        {
//                            // If errno == EAGAIN, that means we have read all data. So go back to the main loop
//                            if(errno != EAGAIN)
//                            {
//                                  GetLog().Msg(Log::Error, "Error in read");
//                                  done = 1;
//                            }
//                            break;
//                        }
//                        else if(count == 0)
//                        {
//                            // End of file. The remote has closed the connection
//                            done = 1;
//                            break;
//                        }

//                        // Write the buffer to standard output
//                        if(write (1, buf, count) == -1)
//                        {
//                            perror ("write");
//                            abort ();
//                        }
//                    }

//                    if (done)
//                    {
//                        printf ("Closed connection on descriptor %d\n", events[i].data.fd);

//                        /* Closing the descriptor will make epoll remove it
//                             from the set of descriptors which are monitored. */
//                        close (events[i].data.fd);
//                    }
                }
            }
        } // for(;;)

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
} // namespace Hps
