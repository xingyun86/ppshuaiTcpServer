// TcpServer.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <unordered_map>
#include <deque>
#include <mutex>
#include <sstream>

// TODO: Reference additional headers your program requires here.
#ifdef _MSC_VER
#include <winsock2.h>
#define SocketFdtype SOCKET
#else
#endif
class TcpServer {
    class ClientData {
    public:
        std::string ip;
        uint16_t port;
        std::stringstream ss;
        std::shared_ptr<std::mutex> locker = std::make_shared<std::mutex>();
        time_t hbtime = 0;
    public:
        ClientData(const std::string& ip, uint16_t port):ip(ip), port(port) {}
    };
    std::unordered_map<SocketFdtype, ClientData> clientList;

    int HeartBeat(SocketFdtype sock)
    {
        if (ReachHeartBeatTime(sock))
        {
            std::string message = "heart beat come!\n";
            send(sock, (const char*)message.data(), message.size(), 0);
            clientList.at(sock).hbtime = time(nullptr);
            printf("%lld\n", clientList.at(sock).hbtime);
        }
        return 0;
    }
    int Process(SocketFdtype sock)
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
        printf("收到客户端<Socket=%d> 数据长度：%d(%.*s)\n", sock, recvLen, recvLen, szRecv);
        clientList.at(sock).locker->lock();
        clientList.at(sock).ss.write(szRecv, recvLen);
        cmd.assign(clientList.at(sock).ss.str());
        if (*cmd.rbegin() == '\n')
        {
            clientList.at(sock).ss.clear();
            clientList.at(sock).ss.str("");
        }
        clientList.at(sock).locker->unlock();
        if (cmd.compare("quit\r\n") == 0)
        {
            printf("客户端<Socket=%d>已主动退出，任务结束...\n", sock);
            closesocket(sock);
            return -1;
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
    bool ReachHeartBeatTime(SocketFdtype sock)
    {
        const static int TIME_HEART_BEAT = 10;
        return ((time(nullptr) - clientList.at(sock).hbtime) >= TIME_HEART_BEAT);
    }
    int Start(const std::string& host, uint16_t port = 18001)
    {
        int nRet = 0;
        // 加载套接字库
        WSADATA wsaData = { 0 };
        // 启动Windows Socket 2.2环境
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            /* Tell the user that we could not find a usable */
            /* WinSock DLL.                                  */
            return 1;
        }

        /* Confirm that the WinSock DLL supports 2.2.*/
        /* Note that if the DLL supports versions greater    */
        /* than 2.2 in addition to 2.2, it will still return */
        /* 2.2 in wVersion since that is the version we      */
        /* requested.                                        */
        if (LOBYTE(wsaData.wVersion) != 2 ||
            HIBYTE(wsaData.wVersion) != 2)
        {
            /* Tell the user that we could not find a usable */
            /* WinSock DLL.                                  */
            WSACleanup();
            return 1;
        }

        /* The WinSock DLL is acceptable. Proceed. */
         //----------------------
        // Create a SOCKET for listening for
        // incoming connection requests.
        SOCKET listenSocket = INVALID_SOCKET;
        listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (listenSocket == INVALID_SOCKET) {
            printf("Error at socket(): %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }
        u_long nOptVal = 1;
        //ioctlsocket(listenSocket, FIONBIO, &nOptVal);
        setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&nOptVal, sizeof(nOptVal));

        //----------------------
        // The sockaddr_in structure specifies the address family,
        // IP address, and port for the socket that is being bound.
        sockaddr_in serverSockAddr = { 0 };
        serverSockAddr.sin_family = AF_INET;
        serverSockAddr.sin_addr.s_addr = inet_addr(host.c_str());
        //serverSockAddr.sin_addr.s_addr = INADDR_ANY;
        serverSockAddr.sin_port = htons(port);

        if (bind(listenSocket, (const sockaddr *)&serverSockAddr, sizeof(serverSockAddr)) == SOCKET_ERROR) {
            printf("bind() failed.绑定网络端口失败(%d)\n", WSAGetLastError());
            closesocket(INVALID_SOCKET);
            WSACleanup();
            return 1;
        }
        else
        {
            printf("绑定网络端口成功...\n");
        }

        //----------------------
        // Listen for incoming connection requests.
        // on the created socket
        if (listen(listenSocket, 5) == SOCKET_ERROR) {
            printf("错误，监听网络端口失败...\n");
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }
        else
        {
            printf("监听网络端口成功...\n");
        }

        printf("等待客户端连接...\n");

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
            FD_SET(listenSocket, &readfds);
            FD_SET(listenSocket, &writefds);
            FD_SET(listenSocket, &exceptfds);

            for (auto& it : clientList)
            {
                FD_SET(it.first, &readfds);
            }

            // 设置超时时间 select 非阻塞
            timeval timeout = { 1, 0 };

            // nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围，而不是数量
            // 即是所有文件描述符最大值+1 在Windows中这个参数可以写0
            int ret = select((int)listenSocket + 1, &readfds, &writefds, &exceptfds, &timeout);
            if (ret < 0)
            {
                printf("select任务结束,called failed:%d!\n", WSAGetLastError());
                break;
            }

            // 是否有数据可读
            // 判断描述符(socket)是否在集合中
            if (FD_ISSET(listenSocket, &readfds))
            {
                //FD_CLR(listenSocket, &readfds);

                // Create a SOCKET for accepting incoming requests.
                // 4. accept 等待接受客户端连接
                sockaddr_in clientSockAddr = { 0 };
                int nclientSockAddrLen = sizeof(sockaddr_in);
                SocketFdtype clientSocket = INVALID_SOCKET;
                clientSocket = accept(listenSocket, (sockaddr*)&clientSockAddr, &nclientSockAddrLen);
                if (INVALID_SOCKET == clientSocket) {
                    printf("accept() failed: %d,接收到无效客户端Socket\n", WSAGetLastError());
                    return 1;
                }
                else
                {
                    // 有新的客户端加入，向之前的所有客户端群发消息
                    for (auto& it : clientList)
                    {
                        std::string message = "hello";
                        send(it.first, (const char*)message.data(), message.size(), 0);
                    }

                    clientList.emplace(clientSocket, ClientData(inet_ntoa(clientSockAddr.sin_addr), ntohs(clientSockAddr.sin_port)));
                    HeartBeat(clientSocket);
                    // 客户端连接成功，则显示客户端连接的IP地址和端口号
                    printf("新客户端<Sokcet=%d>加入,Ip地址：%s,端口号：%d\n",
                        clientSocket,
                        clientList.at(clientSocket).ip.c_str(),
                        clientList.at(clientSocket).port);
                    /*std::string message = "hello\n";
                    send(clientSocket, (const char*)message.data(), message.size(), 0);*/
                }
            }

            for (int i = 0; i < (int)readfds.fd_count; i++)
            {
                if ((readfds.fd_array[i] != listenSocket) 
                    && (Process(readfds.fd_array[i]) == (-1)))
                {
                    auto itFind = clientList.find(readfds.fd_array[i]);
                    if (itFind != clientList.end())
                    {
                        clientList.erase(itFind);
                    }
                }
            }

            for (auto& it : clientList)
            {
                HeartBeat(it.first);
            }
            //printf("空闲时间处理其他业务...\n");
        }

        for (auto& it : clientList)
        {
            closesocket(it.first);
        }

        // 8.关闭套接字
        closesocket(listenSocket);
        // 9.清除Windows Socket环境
        WSACleanup();

        printf("服务端已退出，任务结束\n");

        getchar();

        return 0;
    }

public:
    static TcpServer* Inst() {
        static TcpServer tcpServerInstance;
        return &tcpServerInstance;
    }
};