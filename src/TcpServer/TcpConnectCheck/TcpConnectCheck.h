// TcpConnectCheck.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <string>
#include <shared_mutex>
#include <iostream>
#include <mutex>
#include <sstream>
#include <unordered_map>

// TODO: Reference additional headers your program requires here.
#include <network.h>

class TcpConnectCheck {
public:
    int Start(const std::string& host, uint16_t port = 18001, uint8_t nonblock = 0)
    {
        NET_INIT();

        int nStatus = 0;
        int nSockOpt = 0;
        int nSockOptLen = sizeof(nSockOpt);

        u_long nOptVal = 1;
        char ip[16] = { 0 };
        // Berkeley sockets
        fd_set readfds = { 0 };			// 描述符(socket)集合
        fd_set writefds = { 0 };
        fd_set exceptfds = { 0 };
        struct timeval timeout = { 0 };
        sockaddr_in nameSockAddr = { 0 };
        PPS_SOCKET maxSocket = PPS_INVALID_SOCKET;
        PPS_SOCKET clientSocket = PPS_INVALID_SOCKET;

        /* The WinSock DLL is acceptable. Proceed. */
         //----------------------
        // Create a SOCKET for listening for
        // incoming connection requests.
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (clientSocket == PPS_INVALID_SOCKET) {
            printf("Error at socket(): %d,%s\n", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE).c_str());
            return 1;
        }
        if (nonblock != 0)
        {
            PPS_SetNonBlock(clientSocket, nonblock);
        }
        setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOptVal, sizeof(nOptVal));

        //----------------------
        // Listen for incoming connection requests.
        // on the created socket        
        nameSockAddr.sin_family = AF_INET;
        nameSockAddr.sin_addr.s_addr = inet_addr(host.c_str());
        //nameSockAddr.sin_addr.s_addr = INADDR_ANY;
        nameSockAddr.sin_port = htons(port);
        if (connect(clientSocket, (sockaddr*)&nameSockAddr, sizeof(sockaddr_in)) == PPS_SOCKET_ERROR)
        {
            if (NET_ERR_CODE == PPS_EWOULDBLOCK)
            {
                // 设置超时时间 select 非阻塞
                timeout.tv_sec = 0;
                timeout.tv_usec = 100*1000;

                // 清理集合
                FD_ZERO(&readfds);
                FD_ZERO(&writefds);
                FD_ZERO(&exceptfds);

                // 将描述符(socket)加入集合
                FD_SET(clientSocket, &readfds);
                FD_SET(clientSocket, &writefds);
                FD_SET(clientSocket, &exceptfds);

                nStatus = select((int)clientSocket + 1, &readfds, &writefds, &exceptfds, &timeout);
                if (nStatus < 0)
                {
                    //timeout or select error
                    printf("Connect to server failed...\n");
                    PPS_CloseSocket(clientSocket);
                    return (1);
                }
                else if (nStatus == 0)
                {
                    //timeout or select error
                    printf("Connect to server timeout...\n");
                    PPS_CloseSocket(clientSocket);
                    return (1);
                }
                else
                {
                    getsockopt(clientSocket, SOL_SOCKET, SO_ERROR, (PPS_SOCKOPT_T*)&nSockOpt, (PPS_SOCKLEN_T*)&nSockOptLen);
                    if (nSockOpt != 0)
                    {
                        printf("Connect to server failed... %d\n", nSockOpt);
                        PPS_CloseSocket(clientSocket);
                        return (1);
                    }
                }
            }
            else
            {
                printf("Connect to server failed... %d,%s\n", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE).c_str());
                return (1);
            }
        }

        printf("Connect to server ok!\n");

        // 8.关闭套接字
        PPS_CloseSocket(clientSocket);

        printf("client exited, task completed!\n");

        return 0;
    }

public:
    static TcpConnectCheck* Inst() {
        static TcpConnectCheck tcpConnectCheckInstance;
        return &tcpConnectCheckInstance;
    }
};