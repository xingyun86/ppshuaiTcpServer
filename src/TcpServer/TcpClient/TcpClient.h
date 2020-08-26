// TcpClient.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <mutex>
#include <sstream>

// TODO: Reference additional headers your program requires here.
#include <network.h>

class TcpClient {
   std::shared_ptr<SockData> sd=nullptr;
    int ReqHeartBeat(PPS_SOCKET sock)
    {
        std::string message = "PING\r\n";
        send(sock, (const char*)message.data(), message.size(), 0);
        sd->hbtime = time(nullptr);
        printf("Request Heartbeat%lld\n", sd->hbtime);
        return 0;
    }
    int RespHeartBeat(PPS_SOCKET sock)
    {
        std::string message = "PONG\r\n";
        send(sock, (const char*)message.data(), message.size(), 0);
        sd->hbtime = time(nullptr);
        printf("Request Heartbeat%lld\n", sd->hbtime);
        return 0;
    }
    int Process(PPS_SOCKET sock)
    {
        std::string cmd = ("");
        // 缓冲区(4096字节)
        char szRecv[4096] = {};
        // 5、接收客户端的请求
        // 接收消息
        int recvLen = recv(sock, szRecv, sizeof(szRecv), 0);
        if (recvLen <= 0)
        {
            printf("客户端<Socket=%d>已退出，任务结束...\n", sock);
            return -1;
        }
        printf("收到服务端<Socket=%d> 数据长度：%d(%.*s)\n", sock, recvLen, recvLen, szRecv);
        sd->locker->lock();
        sd->ss.write(szRecv, recvLen);
        cmd.assign(sd->ss.str());
        if (*cmd.rbegin() == '\n')
        {
            sd->ss.clear();
            sd->ss.str("");
        }
        sd->locker->unlock();
        if (cmd.compare("quit\r\n") == 0)
        {
            printf("客户端<Socket=%d>已主动退出，任务结束...\n", sock);
            closesocket(sock);
            return -1;
        }
        if (cmd.compare("PONG\r\n") == 0)
        {
            printf("客户端<Socket=%d>收到心跳回复PONG...\n", sock);
        }
        
        /*DataHeader* pHeader = (DataHeader*)szRecv;
        if (recvLen <= 0)
        {
            printf("客户端<Socket=%d>已退出，任务结束...", sock);
            return -1;
        }

        // 6、处理请求
        switch (pHeader->cmd)
        {
        case CMD_LOGIN:
        {
            Login* login = (Login*)szRecv;

            recv(sock, szRecv + sizeof(DataHeader), pHeader->dataLength - sizeof(DataHeader), 0);
            printf("收到客户端<Socket=%d>请求：CMD_LOGIN, 数据长度：%d, userName：%s Password： %s\n",
                sock, login->dataLength, login->userName, login->passWord);
            // 忽略判断用户名和密码是否正确的过程
            LoginResult ret;
            send(sock, (char*)&ret, sizeof(LoginResult), 0);
        }
        break;
        case CMD_LOGOUT:
        {
            Logout* logout = (Logout*)szRecv;

            recv(sock, szRecv + sizeof(DataHeader), pHeader->dataLength - sizeof(DataHeader), 0);
            printf("收到客户端<Socket=%d>请求：CMD_LOGOUT, 数据长度：%d, userName：%s\n",
                sock, logout->dataLength, logout->userName);
            LogoutResult ret;
            send(sock, (char*)&ret, sizeof(LogoutResult), 0);
        }
        break;
        default:
        {
            DataHeader header = { 0, CMD_ERROR };
            send(sock, (char*)&header, sizeof(header), 0);
            break;
        }
        }*/

        return 0;
    }

public:
    long TIMER_HEART_BEAT = 20;
    bool Timeout(std::time_t t, long time)
    {
        return ((std::time(nullptr) - t) > time);
    }
    int Start(const std::string& host, uint16_t port = 18001)
    {
        NET_INIT();

        /* The WinSock DLL is acceptable. Proceed. */
         //----------------------
        // Create a SOCKET for listening for
        // incoming connection requests.
        PPS_SOCKET clientSocket = PPS_INVALID_SOCKET;
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (clientSocket == PPS_INVALID_SOCKET) {
            printf("Error at socket(): %ld\n", WSAGetLastError());
            return 1;
        }
        u_long nOptVal = 1;
        //ioctlsocket(listenSocket, FIONBIO, &nOptVal);
        setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOptVal, sizeof(nOptVal));

        //----------------------
        // Listen for incoming connection requests.
        // on the created socket        
        sockaddr_in serverSockAddr = { 0 };
        serverSockAddr.sin_family = AF_INET;
        serverSockAddr.sin_addr.s_addr = inet_addr(host.c_str());
        //serverSockAddr.sin_addr.s_addr = INADDR_ANY;
        serverSockAddr.sin_port = htons(port);

        if (connect(clientSocket, (sockaddr*)&serverSockAddr, sizeof(sockaddr_in)) == SOCKET_ERROR) {
            printf("错误，连接服务器失败...\n");
            PPS_CloseSocket(clientSocket);
            return 1;
        }
        else
        {
            printf("连接服务器成功...\n");
        }

        printf("客户端连接服务器成功...\n");

        sd = std::make_shared<SockData>(host, port);

        while (true)
        {
            // Berkeley sockets
            fd_set readfds = { 0 };			// 描述符(socket)集合
            fd_set writefds = { 0 };
            fd_set exceptfds = { 0 };

            // 清理集合
            FD_ZERO(&readfds);
            FD_ZERO(&writefds);
            FD_ZERO(&exceptfds);

            // 将描述符(socket)加入集合
            FD_SET(clientSocket, &readfds);
            FD_SET(clientSocket, &writefds);
            FD_SET(clientSocket, &exceptfds);

            // 设置超时时间 select 非阻塞
            timeval timeout = { 1, 0 };

            // nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围，而不是数量
            // 即是所有文件描述符最大值+1 在Windows中这个参数可以写0
            int ret = select((int)clientSocket + 1, &readfds, &writefds, &exceptfds, &timeout);
            if (ret < 0)
            {
                printf("select任务结束,called failed:%d!\n", WSAGetLastError());
                break;
            }

            // 是否有数据可读
            // 判断描述符(socket)是否在集合中
            if (FD_ISSET(clientSocket, &readfds))
            {
                FD_CLR(clientSocket, &readfds);
                if (Process(clientSocket) == (-1))
                {
                    break;
                }
            }

            if (Timeout(sd->hbtime, TIMER_HEART_BEAT))
            {
                ReqHeartBeat(clientSocket);
            }

            //printf("空闲时间处理其他业务...\n");
        }

        // 8.关闭套接字
        PPS_CloseSocket(clientSocket);

        printf("客户端已退出，任务结束\n");

        getchar();

        return 0;
    }

public:
    static TcpClient* Inst() {
        static TcpClient tcpClientInstance;
        return &tcpClientInstance;
    }
};